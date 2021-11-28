#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "main.h"
#include "List.h"
#include "ActionFunctors.h"
#include "SystemEvents.h"

namespace Listener {
  class Abstract {
   public:
    virtual ~Abstract() = default;
    virtual void ProcessSystemEvent(const SystemEvent& event) = 0;
  };

  class Drag : public Abstract {
   public:
    Drag() = delete;
    Drag(Functor::MoveWidget* move_func, Widget::Drag* widget_drag);
    ~Drag() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Functor::MoveWidget* move_func_;
    Widget::Drag* widget_drag_;
  };

  class ButtonClick : public Abstract {
   public:
    ButtonClick() = delete;
    ButtonClick(Functor::Abstract* action_func, Widget::Button* button);
    ~ButtonClick() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Functor::Abstract* action_func_;
    Widget::Button* button_;
  };

  class ButtonHover : public Abstract {
   public:
    ButtonHover() = delete;
    ButtonHover(Widget::Button* button);
    ~ButtonHover() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Widget::Button* button_;
  };

  class Canvas : public Abstract {
   public:
    Canvas() = delete;
    Canvas(Widget::Canvas* canvas,
           PluginTexture* painting_area,
           Point2D<uint> coord);
    ~Canvas() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Widget::Canvas* canvas_;
    PluginTexture* painting_area_;
    Tool::Manager& manager_;
    bool is_in_action_;
  };
}

namespace Widget {
  class Abstract {
   public:
    Abstract() = delete;
    Abstract(const Rectangle& position,
             DrawFunctor::Abstract* draw_func);
    virtual ~Abstract() = default;

    Rectangle GetPosition();
    void SetDrawFunc(DrawFunctor::Abstract* draw_func);
    virtual Point2D<int> Move(const Point2D<int>& shift,
                              const Rectangle& bounds);

    virtual void Resize(const Point2D<int>& corner_shift,
                        int width_shift, int height_shift,
                        const Rectangle& bounds);

    virtual bool IsMouseCoordinatesInBound(const Point2D<uint>& mouse_coordinates);
    virtual void Draw();
    virtual void ProcessSystemEvent(const SystemEvent& event) = 0;

   protected:
    Rectangle position_;
    DrawFunctor::Abstract* draw_func_;
  };

  class AbstractContainer : public Abstract {
   public:
    AbstractContainer() = delete;
    AbstractContainer(const Rectangle& position,
                      std::initializer_list<Widget::Abstract*> children,
                      DrawFunctor::Abstract* draw_func);
    ~AbstractContainer() override;

    void DeleteChildren();
    void DrawChildren();
    List<Widget::Abstract*>& GetChildren();
    void AddChild(Widget::Abstract* widget);
    void PushMouseUpToChildInFocus(const SystemEvent& event);
    void PushMouseMotionToChildInFocus(const SystemEvent& event);
    void PushMouseDownToChildInFocusAndTopHim(const SystemEvent& event);

    void Draw() override;
    Point2D<int> Move(const Point2D<int>& shift,
                      const Rectangle& bounds) override;

   protected:
    List<Widget::Abstract*> children_;
  };

  class MainWindow : public AbstractContainer {
   public:
    MainWindow() = delete;
    MainWindow(const Rectangle& position,
               std::initializer_list<Widget::Abstract*> children,
               DrawFunctor::Abstract* draw_func);
    ~MainWindow() override = default;

    void AddListener(SystemEvent::Type event_type, Listener::Abstract* listener);
    void DeleteListener(SystemEvent::Type event_type, Listener::Abstract* listener);
    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    std::unordered_map<SystemEvent::Type, std::unordered_set<Listener::Abstract*> > listener_table_;
    std::unordered_set<Listener::Abstract*> processed_listeners_;

    bool IsListenerProcessed(Listener::Abstract* listener);
    void SendEventToListeners(const SystemEvent& event);
  };

  class Container : public AbstractContainer {
   public:
    Container() = delete;
    Container(const Rectangle& position,
              std::initializer_list<Widget::Abstract*> children,
              DrawFunctor::Abstract* draw_func);
    ~Container() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;
  };

  class Drag : public AbstractContainer {
   public:
    Drag() = delete;
    Drag(const Rectangle& position,
         Widget::MainWindow* main_window,
         std::initializer_list<Widget::Abstract*> children,
         Functor::MoveWidget* move_func,
         DrawFunctor::Abstract* draw_func);
    ~Drag() override;

    void StartDrag();
    void FinishDrag();
    void ProcessSystemEvent(const SystemEvent& event) override;

   protected:
    Widget::MainWindow* main_window_;
    Functor::MoveWidget* move_func_;
    Listener::Drag* drag_listener_;
  };

  class Button : public Abstract {
   public:
    struct DrawFunctors {
      DrawFunctor::Abstract* draw_func_main = nullptr;
      DrawFunctor::Abstract* draw_func_hover = nullptr;
      DrawFunctor::Abstract* draw_func_click = nullptr;
    };

    Button() = delete;
    Button(const Rectangle& position,
           Widget::MainWindow* main_window,
           Functor::Abstract* action_func,
           const DrawFunctors& draw_funcs);
    ~Button() override;

    void StartListeningMouseUp();
    void StartListeningMouseMotion();
    void StopListeningMouseUp();
    void StopListeningMouseMotion();
    void ProcessSystemEvent(const SystemEvent& event) override;

   protected:
    Widget::MainWindow* main_window_;
    Functor::Abstract* action_func_;
    DrawFunctors draw_funcs_;
    Listener::ButtonClick* click_listener_;
    Listener::ButtonHover* hover_listener_;
  };

  class Canvas : public Abstract {
   public:
    Canvas() = delete;
    Canvas(const Rectangle& position,
           Widget::MainWindow* main_window,
           Render* render);
    ~Canvas() override;

    void StartPainting(Point2D<uint> coord);
    void FinishPainting();
    void ProcessSystemEvent(const SystemEvent& event) override;
    void Draw() override;

   protected:
    Widget::MainWindow* main_window_;
    PluginTexture* painting_area_;
    Listener::Canvas* painting_listener_;
  };
}

namespace UserWidget {
  class StandardWindow : public Widget::Container {
   public:
    StandardWindow() = delete;
    StandardWindow(const Rectangle& pos,
                   Widget::MainWindow* main_window);
    ~StandardWindow() override;

   private:
    Functor::CloseWidget* func_close_widget_;
    Functor::MoveWidget* func_move_;
  };

  class PaintWindow : public StandardWindow {
   public:
    PaintWindow() = delete;
    PaintWindow(const Rectangle& pos,
                Widget::MainWindow* main_window,
                Render* render);
    ~PaintWindow() override;

   private:
    struct Tool {
      Texture* texture;
    };
    std::vector<Texture*> textures_to_free_;
    std::vector<DrawFunctor::Abstract*> draw_funcs_to_free_;
    std::vector<Functor::PickColor*> pick_color_funcs_to_free_;
    std::vector<Functor::SetTool*> set_tool_funcs_to_free_;

    void CreatePalette(Widget::Container* palette, Render* render, uint palette_width,
                       const Point2D<int>& coord, Widget::MainWindow* main_window);
    Widget::Button* CreateColorButton(Render* render, uint button_width,
                                      const Point2D<int>& coord, Widget::MainWindow* main_window);
  };

  class HoleWindow : public Widget::Drag {
   public:
    HoleWindow() = delete;
    HoleWindow(const Rectangle& position,
               Widget::MainWindow* main_window,
               Render* render);
    ~HoleWindow() override;
    void Draw() override;
    bool IsMouseCoordinatesInBound(const Point2D<uint>& mouse_coordinates) override;

   private:
    Texture* texture_;

    bool IsInBound(const Point2D<int>& mouse_coord);
  };

  class ButtonWithText : public Widget::Button {
   public:
    ButtonWithText() = delete;
    ButtonWithText(const Rectangle& position,
                   Widget::MainWindow* main_window,
                   Functor::Abstract* action_func,
                   const DrawFunctors& draw_funcs,
                   const char* text,
                   Render* render,
                   const Color& color);
    ButtonWithText(const Point2D<int>& position,
                   Widget::MainWindow* main_window,
                   Functor::Abstract* action_func,
                   const DrawFunctors& draw_funcs,
                   const char* text,
                   Render* render,
                   const Color& color);
    ~ButtonWithText() override;

   protected:
    Texture* text_;
    DrawFunctor::TextTexture* draw_text_;
  };
}