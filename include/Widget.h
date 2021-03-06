#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "main.h"
#include "List.h"
#include "SystemEvents.h"
#include "ActionFunctors.h"
#include "FunctorQueue.h"
#include "GUIConstants.h"

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

  class BasicButtonClick : public Abstract {
   public:
    BasicButtonClick() = delete;
    BasicButtonClick(Functor::Abstract* action_func, Widget::BasicButton* button);
    ~BasicButtonClick() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Functor::Abstract* action_func_;
    Widget::BasicButton* button_;
  };

  class BasicButtonHover : public Abstract {
   public:
    BasicButtonHover() = delete;
    BasicButtonHover(Widget::BasicButton* button);
    ~BasicButtonHover() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Widget::BasicButton* button_;
  };

  class ButtonOnPress : public Abstract {
   public:
    ButtonOnPress() = delete;
    ButtonOnPress(Widget::ButtonOnPress* button);
    ~ButtonOnPress() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Widget::ButtonOnPress* button_;
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
    void SetPosition(const Rectangle& pos);
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

  class Icon : public Abstract {
   public:
    Icon() = delete;
    Icon(const Rectangle& position,
         DrawFunctor::Abstract* draw_func)
    : Abstract(position, draw_func) {}

    void ProcessSystemEvent(const SystemEvent& event) override {}
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
    std::list<Widget::Abstract*>& GetChildren();
    void AddChild(Widget::Abstract* widget);
    void PushMouseUpToChildInFocus(const SystemEvent& event);
    void PushMouseMotionToChildInFocus(const SystemEvent& event);
    void PushMouseDownToChildInFocusAndTopHim(const SystemEvent& event);

    void Draw() override;
    Point2D<int> Move(const Point2D<int>& shift,
                      const Rectangle& bounds) override;

   protected:
    std::list<Widget::Abstract*> children_;
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

  struct ButtonDrawFunctors {
    DrawFunctor::Abstract* draw_func_main = nullptr;
    DrawFunctor::Abstract* draw_func_hover = nullptr;
    DrawFunctor::Abstract* draw_func_click = nullptr;
  };

  class BasicButton : public Abstract {
   public:
    BasicButton() = delete;
    BasicButton(const Rectangle& position,
                Widget::MainWindow* main_window,
                Functor::Abstract* action_func,
                const ButtonDrawFunctors& draw_funcs);
    ~BasicButton() override;

    ButtonDrawFunctors GetDrawFuncs();
    void ProcessSystemEvent(const SystemEvent& event) override;
    void Action();

    friend Listener::BasicButtonClick;
    friend Listener::BasicButtonHover;

   protected:
    Widget::MainWindow* main_window_;
    Functor::Abstract* action_func_;
    ButtonDrawFunctors draw_funcs_;
    Listener::BasicButtonClick* click_listener_;
    Listener::BasicButtonHover* hover_listener_;

    void StartListeningMouseUp();
    void StartListeningMouseMotion();
    void StopListeningMouseUp();
    void StopListeningMouseMotion();
  };

  class ButtonOnPress : public Abstract {
   public:
    ButtonOnPress() = delete;
    ButtonOnPress(const Rectangle& position,
                  Widget::MainWindow* main_window,
                  Functor::Abstract* action_func,
                  const ButtonDrawFunctors& draw_funcs);
    ~ButtonOnPress() override;

    void ProcessSystemEvent(const SystemEvent& event) override;
    void StopTheClick();

    friend Listener::ButtonOnPress;

   protected:
    Widget::MainWindow* main_window_;
    Functor::Abstract* action_func_;
    ButtonDrawFunctors draw_funcs_;
    Listener::ButtonOnPress* hover_listener_;
    bool is_in_click_state_;

    void StartListeningMouseMotion();
    void StopListeningMouseMotion();
  };
}

namespace UserWidget {
  class StandardWindow : public Widget::Container {
   public:
    StandardWindow() = delete;
    StandardWindow(const Rectangle& pos,
                   Widget::MainWindow* main_window);
    ~StandardWindow() override;

    void Close();

   private:
    Functor::CloseWidget* func_close_widget_;
    Functor::MoveWidget* func_move_;
    Widget::BasicButton* button_close_;
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

  struct ButtonDrawInfo {
    Widget::ButtonDrawFunctors draw_funcs;
    Render* render;
    Color text_color;
  };

  class BasicButtonWithText : public Widget::BasicButton {
   public:
    BasicButtonWithText() = delete;
    BasicButtonWithText(const Rectangle& position,
                        Widget::MainWindow* main_window,
                        Functor::Abstract* action_func,
                        const ButtonDrawInfo& button_draw_info,
                        const char* text);

    BasicButtonWithText(const Point2D<int>& position,
                        Widget::MainWindow* main_window,
                        Functor::Abstract* action_func,
                        const ButtonDrawInfo& button_draw_info,
                        const char* text);

    ~BasicButtonWithText() override;

   protected:
    Texture* text_;
    DrawFunctor::TextTexture* draw_text_;
  };

  class ButtonOnPressWithText : public Widget::ButtonOnPress {
   public:
    ButtonOnPressWithText(const Point2D<int>& position,
                          Widget::MainWindow* main_window,
                          Functor::Abstract* action_func,
                          const ButtonDrawInfo& button_draw_info,
                          const char* text);

    ~ButtonOnPressWithText() override;

   protected:
    Texture* text_;
    DrawFunctor::TextTexture* draw_text_;
  };

  class Label : public Widget::Abstract {
   public:
    Label() = delete;
    Label(const Point2D<int>& position,
          const char* text,
          Render* render,
          const Color& color = {});
    ~Label() override;

    void SetText(const char* text);
    void ProcessSystemEvent(const SystemEvent& event) override {}   

   protected:
    Render* render_;
    Texture* text_;
    DrawFunctor::TextTexture* draw_text_;
  };
}