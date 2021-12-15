#pragma once
#include "Widget.h"

namespace Listener {
  class ScrollHover : public Abstract {
   public:
    ScrollHover() = delete;
    ScrollHover(Widget::Scroll* widget_scroll);
    ~ScrollHover() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Widget::Scroll* widget_scroll_;
  };

  class Scroll : public Abstract {
   public:
    Scroll() = delete;
    Scroll(Functor::Scroll* scroll_func,
           Widget::Scroll* widget_scroll,
           ScrollType scroll_type,
           int bound0,
           int bound1);
    ~Scroll() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Functor::Scroll* scroll_func_;
    Widget::Scroll* widget_scroll_;
    ScrollType scroll_type_;
    int bound0_;
    int bound1_;
  };
}

namespace Widget {
  class Scroll : public Abstract {
   public:
    Scroll() = delete;
    Scroll(const Rectangle& position,
           Widget::MainWindow* main_window,
           ScrollType scroll_type,
           int bound0,
           int bound1,
           Functor::Scroll* scroll_func,
           const ButtonDrawFunctors& draw_funcs);
    ~Scroll() override;

    void SetBound0(int bound0);
    void SetBound1(int bound1);
    Point2D<int> Move(const Point2D<int>& shift,
                      const Rectangle& bounds) override;
    void ProcessSystemEvent(const SystemEvent& event) override;
    friend Listener::Scroll;
    friend Listener::ScrollHover;

   protected:
    Widget::MainWindow* main_window_;
    ScrollType scroll_type_;
    int bound0_;
    int bound1_;
    Functor::Scroll* scroll_func_;
    ButtonDrawFunctors draw_funcs_;
    Listener::ScrollHover* hover_listener_;
    Listener::Scroll* scroll_listener_;
    
    void StartHovering();
    void StopHovering();
    void StartScroll();
    void FinishScroll();
  };
}