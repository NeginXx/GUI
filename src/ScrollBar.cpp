#include "../include/ScrollBar.h"

namespace Listener {
  ScrollHover::ScrollHover(Widget::Scroll* widget_scroll)
  : widget_scroll_(widget_scroll) {}

  void ScrollHover::ProcessSystemEvent(const SystemEvent& event) {
    assert(event.type == SystemEvent::kMouseMotion);
    if (!widget_scroll_->IsMouseCoordinatesInBound(event.info.mouse_motion.new_mouse_pos)) {
      widget_scroll_->StopHovering();
    }
  }

  Scroll::Scroll(Functor::Scroll* scroll_func,
                 Widget::Scroll* widget_scroll,
                 ScrollType scroll_type,
                 int bound0,
                 int bound1)
  : scroll_func_(scroll_func), widget_scroll_(widget_scroll),
    scroll_type_(scroll_type),
    bound0_(bound0), bound1_(bound1) {}

  void Scroll::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonUp: {
        widget_scroll_->FinishScroll();
        break;
      }

      case SystemEvent::kMouseMotion: {
        auto info = event.info.mouse_motion;
        int x_diff = (int)info.new_mouse_pos.x - (int)info.old_mouse_pos.x;
        int y_diff = (int)info.new_mouse_pos.y - (int)info.old_mouse_pos.y;

        Rectangle pos = widget_scroll_->GetPosition();
        if (scroll_type_ == ScrollType::kHorizontal) {
          pos.corner.x = Max(bound0_, Min(pos.corner.x + x_diff, bound1_ - (int)pos.width));
          widget_scroll_->SetPosition(pos);
          scroll_func_->SetScrollPos((float)(pos.corner.x - bound0_) / (float)(bound1_ - bound0_ - pos.width));
          FunctorQueue::GetInstance().Push(scroll_func_);
        } else if (scroll_type_ == ScrollType::kVertical) {
          pos.corner.y = Max(bound0_, Min(pos.corner.y + y_diff, bound1_ - (int)pos.height));
          widget_scroll_->SetPosition(pos);
          scroll_func_->SetScrollPos((float)(pos.corner.y - bound0_) / (float)(bound1_ - bound0_ - pos.height));
          FunctorQueue::GetInstance().Push(scroll_func_);
        }

        break;
      }

      default: assert(0);
    }
  }
}

namespace Widget {
  Scroll::Scroll(const Rectangle& position,
                 Widget::MainWindow* main_window,
                 ScrollType scroll_type,
                 int bound0,
                 int bound1,
                 Functor::Scroll* scroll_func,
                 const ButtonDrawFunctors& draw_funcs)
  : Abstract(position, draw_funcs.draw_func_main), main_window_(main_window),
    scroll_type_(scroll_type),
    bound0_(bound0), bound1_(bound1),
    scroll_func_(scroll_func), draw_funcs_(draw_funcs),
    hover_listener_(nullptr), scroll_listener_(nullptr)
  {
    assert(bound0 <= bound1);
    if (scroll_func_ != nullptr) {
      scroll_func_->SetScrollType(scroll_type_);
    }
  }

  Scroll::~Scroll() {
    if (hover_listener_ != nullptr) {
      StopHovering();
    }
    if (scroll_listener_ != nullptr) {
      FinishScroll();
    }
  }

  void Scroll::SetBound0(int bound0) {
    bound0_ = bound0;
  }

  void Scroll::SetBound1(int bound1) {
    bound1_ = bound1;
  }

  void Scroll::StartHovering() {
    hover_listener_ = new Listener::ScrollHover(this);
    main_window_->AddListener(SystemEvent::kMouseMotion, hover_listener_);
    if (draw_funcs_.draw_func_hover != nullptr) {
      draw_func_ = draw_funcs_.draw_func_hover;
    }
  }

  void Scroll::StopHovering() {
    main_window_->DeleteListener(SystemEvent::kMouseMotion, hover_listener_);
    delete hover_listener_;
    hover_listener_ = nullptr;
    if (draw_func_ == draw_funcs_.draw_func_hover) {
      draw_func_ = draw_funcs_.draw_func_main;
    }
  }

  void Scroll::StartScroll() {
    scroll_listener_ = new Listener::Scroll(scroll_func_, this, scroll_type_, bound0_, bound1_);
    main_window_->AddListener(SystemEvent::kMouseMotion, scroll_listener_);
    main_window_->AddListener(SystemEvent::kMouseButtonUp, scroll_listener_);
    if (draw_funcs_.draw_func_click != nullptr) {
      draw_func_ = draw_funcs_.draw_func_click;
    }
  }

  void Scroll::FinishScroll() {
    main_window_->DeleteListener(SystemEvent::kMouseMotion, scroll_listener_);
    main_window_->DeleteListener(SystemEvent::kMouseButtonUp, scroll_listener_);
    delete scroll_listener_;
    scroll_listener_ = nullptr;
    if (draw_func_ == draw_funcs_.draw_func_click) {
      draw_func_ = draw_funcs_.draw_func_main;
    }
  }

  void Scroll::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonDown: {
        StartScroll();
        break;
      }

      case SystemEvent::kMouseMotion: {
        if (hover_listener_ == nullptr) {
          if (scroll_listener_ == nullptr) {
            StartHovering();
          }
        } // else it's already processed by listener
        break;
      }
    }
  }

  Point2D<int> Scroll::Move(const Point2D<int>& shift,
                            const Rectangle& bounds) {
    Point2D<int> real_shift = Abstract::Move(shift, bounds);
    if (scroll_type_ == kHorizontal) {
      bound0_ += real_shift.x;
      bound1_ += real_shift.x;
    } else if (scroll_type_ == kVertical) {
      bound0_ += real_shift.y;
      bound1_ += real_shift.y;
    }
    return real_shift;
  }
}