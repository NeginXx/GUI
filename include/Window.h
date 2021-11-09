#pragma once
#include "main.h"
#include "List.h"
#include "ActionCommands.h"
#include "SystemEvents.h"

namespace Widget {
  class Abstract {
   public:
    Abstract() = delete;
    Abstract(const Rectangle<size_t>& position)
    : position_(position) {};
    virtual ~Abstract() = default;

    bool IsMouseCoordinatesInBound(const Point2D<size_t>& parent_coordinates,
                                   const Point2D<size_t>& mouse_coordinates);
    void Move(int x_diff, int y_diff);
    void Resize(int width_diff, int height_diff);
    virtual void Draw(const Point2D<size_t>& parent_coordinates) = 0;
    virtual void ProcessSystemEvent(const Point2D<size_t>& parent_coordinates,
                                    const SystemEvent& event) = 0;

   protected:
    Rectangle<size_t> position_;
  };

  class Window : public Abstract {
   public:
    Window() = delete;
    Window(const Rectangle<size_t>& position,
           std::initializer_list<Widget::Abstract*> children,
           DrawFunctor::Abstract* draw_func)
    : Abstract(position), children_(children),
      draw_func_(draw_func) {};
    ~Window() override;

    List<Widget::Abstract*>& GetChildren();
    void AddChild(Widget::Abstract* widget);
    void Draw(const Point2D<size_t>& parent_coordinates) override;
    void ProcessSystemEvent(const Point2D<size_t>& parent_coordinates,
                            const SystemEvent& event) override;

   private:
    List<Widget::Abstract*> children_;
    DrawFunctor::Abstract* draw_func_;
  };

  class Resize : public Abstract {
   public:
    Resize() = delete;
    Resize(const Rectangle<size_t>& position,
           ResizeFunctor::Abstract* resize_func,
           DrawFunctor::Abstract* draw_func)
    : Abstract(position),
      resize_func_(resize_func),
      draw_func_(draw_func) {};
    ~Resize() override = default;

    void Draw(const Point2D<size_t>& parent_coordinates) override;
    void ProcessSystemEvent(const Point2D<size_t>& parent_coordinates,
                            const SystemEvent& event) override;

   private:
    ResizeFunctor::Abstract* resize_func_;
    DrawFunctor::Abstract* draw_func_;
  };

  class TitleBar : public Abstract {
   public:
    TitleBar() = delete;
    TitleBar(const Rectangle<size_t>& position,
             std::initializer_list<Widget::Abstract*> children,
             MoveFunctor::Abstract* move_func,
             DrawFunctor::Abstract* draw_func)
    : Abstract(position), children_(children),
      move_func_(move_func), draw_func_(draw_func) {}
    ~TitleBar() override;

    void Draw(const Point2D<size_t>& parent_coordinates) override;
    void ProcessSystemEvent(const Point2D<size_t>& parent_coordinates,
                            const SystemEvent& event) override;

   private:
    List<Widget::Abstract*> children_;
    MoveFunctor::Abstract* move_func_;
    DrawFunctor::Abstract* draw_func_;
  };

  class Button : public Abstract {
   public:
    Button() = delete;
    Button(const Rectangle<size_t>& position,
           ActionFunctor::Abstract* action_func,
           DrawFunctor::Abstract* draw_func)
    : Abstract(position),
      action_func_(action_func),
      draw_func_(draw_func) {};
    ~Button() override = default;

    void Draw(const Point2D<size_t>& parent_coordinates) override;
    void ProcessSystemEvent(const Point2D<size_t>& parent_coordinates,
                            const SystemEvent& event) override;

   private:
    ActionFunctor::Abstract* action_func_;
    DrawFunctor::Abstract* draw_func_;
  };

  class Canvas : public Abstract {
   public:
    Canvas() = delete;
    Canvas(const Rectangle<size_t>& position,
           CanvasFunctor* canvas_func)
    : Abstract(position),
      canvas_func_(canvas_func) {};

    void Draw(const Point2D<size_t>& parent_coordinates) override;
    void ProcessSystemEvent(const Point2D<size_t>& parent_coordinates,
                            const SystemEvent& event) override;

   private:
    CanvasFunctor* canvas_func_;
  };
}