#include "../include/Window.h"

extern std::queue<Functor::Abstract*> kFunctorQueue;

namespace Listener {
  class Abstract {
   public:
    virtual void ProcessSystemEvent(const SystemEvent& event) = 0;
  };

  class TitleBar {
   public:
    TitleBar() = delete;
    TitleBar(Functor::MoveWidget* move_func, Widget::TitleBar* title_bar)
    : move_func_(move_func), title_bar_(title_bar) {}

    virtual void ProcessSystemEvent(const SystemEvent& event) override {
      switch (event.type) {
        case SystemEvent::kMouseButtonUp: {
          title_bar_->FinishDrag();
          break;
        }

        case SystemEvent::kMouseMotion: {
          auto info = event.info.mouse_motion;
          int x_diff = (int)info.new_mouse_pos.x - (int)info.old_mouse_pos.x;
          int y_diff = (int)info.new_mouse_pos.y - (int)info.old_mouse_pos.y;
          move_func_.SetShift(Point2D<int>{x_diff, y_diff});
          kFunctorQueue.push(move_func_);    
        }

        default: assert(0);
      }
    }

   private:
    Functor::MoveWidget* move_func_;
    Widget::TitleBar title_bar_;
  };

  class Button {
   public:
    Button() = delete;
    Button(Functor::Abstract* func, Widget::Button* button)
    : func_(func), button_(button) {}

    virtual void ProcessSystemEvent(const SystemEvent& event) override {
      assert(event.type == SystemEvent::kMouseUp);
      kFunctorQueue.push(func_);
      button_->StopListeningMouseUp();
    }

   private:
    Functor::Abstract* func_;
    Widget::Button* button_;
  };
}

class Abstract {
 public:
  Abstract() = delete;
  Abstract(const Rectangle<size_t>& position,
           DrawFunctor::Abstract* draw_func)
  : position_(position), draw_func_(draw_func) {}
  virtual ~Abstract() = { if (draw_func_ != nullptr) delete draw_func_; }

  virtual void Move(const Point2D<int>& shift,
                    const Rectangle<size_t>& bounds) {
    Point2D<size_t> min_coords = bounds.corner;
    Point2D<size_t> max_coords = bounds.corner + Point2D<size_t>{bounds.width, bounds.height};
    if (shift.x >= 0) {
      size_t bound_dist = (size_t)Max(0, (int)max_coords.x - (int)(position_.x + position_.width));
      position_.x += (size_t)Min(bound_dist, (size_t)shift.x);
    } else {
      size_t bound_dist = (size_t)Max(0, (int)position_.x - (int)min_coords.x);
      size_t shift_positive = (size_t)(-shift.x);
      position.x -= (size_t)Min(bound_dist, shift_positive);
    }

    if (shift.y >= 0) {
      size_t bound_dist = (size_t)Max(0, (int)max_coords.y - (int)(position_.y + position_.height));
      position_.y += (size_t)Min(bound_dist, (size_t)shift.y);
    } else {
      size_t bound_dist = (size_t)Max(0, (int)position_.y - (int)min_coords.y);
      size_t shift_positive = (size_t)(-shift.y);
      position.y -= (size_t)Min(bound_dist, shift_positive);
    }
  }

  virtual void Resize(const Point2D<int>& corner_shift,
                      int width_shift, int height_shift,
                      const Rectangle<size_t>& bounds) {
    if (corner_shift.x >= 0) {
      if (width_shift >= 0) {
        size_t bound_dist = (size_t)Max(0, (int)max_coords.x - (int)(position_.x + position_.width));
        position_.width += width_shift;
      } else {
        position_.corner.x += corner_shift.x;
      }
    }
  }

  virtual bool IsMouseCoordinatesInBound(const Point2D<size_t>& mouse_coordinates) {
    const Point2D<size_t>& m_c = mouse_coordinates;
    Rectangle<size_t> pos = position_;
    return pos.corner.x <= m_c.x && m_c.x <= pos.corner.x + pos.width &&
           pos.corner.y <= m_c.y && m_c.y <= pos.corner.y + pos.height;
  }

  virtual void Draw() {
    if (draw_func_ != nullptr) {
      draw_func_->Action();
    }
  }

  virtual bool ProcessSystemEvent(const SystemEvent& event) = 0;

 private:
  Rectangle<size_t> position_;
  DrawFunctor::Abstract* draw_func_;
};

class Container : public Abstract {
 public:
  Container() = delete;
  Container(const Rectangle<size_t>& position,
            std::initializer_list<Widget::Abstract*> children,
            DrawFunctor::Abstract* draw_func)
  : Abstract(position, draw_func), children_(children) {}

  ~Container() override {
    DeleteChildren();
  }

  void DeleteChildren() {
    for (auto child : children_) {
      delete child;
    }
  }

  void DrawChildren() {
    if (children_.GetSize() != 0) {
      for (auto it = (children_.end()).Prev(); it != children_.begin(); --it) {
        (*it)->Draw();
      }
      (*children_.begin())->Draw();
    }
  }

  #define PUSH_EVENT(event_info)          \
    for (auto child : children_) {        \
      if (child->IsMouseCoordinatesInBound(event.info.event_info)) { \
        child->ProcessSystemEvent(event); \
        break;                            \
      }                                   \
    } 

    void PushMouseUpToChildInFocus(const SystemEvent& event) {
      PUSH_EVENT(mouse_click.coordinate);
    }

    void PushMouseMotionToChildInFocus(const SystemEvent& event) {
      PUSH_EVENT(mouse_motion.new_mouse_pos);
    }


  void PushMouseDownToChildInFocusAndTopHim(const SystemEvent& event) {
    if (children_->GetSize() == 1) {
      PUSH_EVENT(mouse_click.coordinate);
      return;
    }

    for (auto child_it = children_.begin(); child_it != children_.end(); ++child_it) {
      if ((*child_it)->IsMouseCoordinatesInBound(event.info.mouse_click.coordinate)) {
        Widget::Abstract* child = *child_it;
        child->ProcessSystemEvent(event);
        if (child_it != children_.begin()) {
          children_.Pop(child_it);
          children_.PushFront(child);
        }
        break;
      }
    }
  }

  #undef PUSH_EVENT

  virtual void Draw() {
    this->Abstract::Draw();
    DrawChildren();
  }

  virtual void Move(const Point2D<int>& shift,
                    const Rectangle<size_t>& bounds) override {
    this->Abstract::Move(shift, bounds);
    for (auto child : children) {
      child->Move(shift, bounds);
    }
  }

 private:
  List<AbstractWidget*> children_;
};

class MainWindow : public Container {
 public:
  MainWindow() = delete;
  MainWindow(const Rectangle<size_t>& position,
             std::initializer_list<Widget::Abstract*> children,
             DrawFunctor::Abstract* draw_func)
  : Container(position, children, draw_func),
    listener_table_[SystemEvent::kMouseButtonUp],
    listener_table_[SystemEvent::kMouseButtonDown],
    listener_table_[SystemEvent::kMouseMotion] {}

  void AddListener(SystemEvent::Type event_type, Listener::Abstract* listener) {
    auto it = listener_table_.find(event_type);
    assert(it != listener_table_.end());
    listener_table_[event_type].insert(listener);
  }

  void DeleteListener(SystemEvent::Type event_type, Widget::Listener* listener) {
    auto it = listener_table_.find(event_type);
    assert(it != listener_table_.end());
    auto set = listener_table_[event_type];
    if (set.find(listener) == set.end()) {
      assert(0);
    } else {
      set.erase(listener);
    }
  }

  bool ProcessSystemEvent(const SystemEvent& event) override {
    bool is_consumed = false;
    switch (event.type) {
      case SystemEvent::kMouseButtonUp:
        for (auto listener : listener_table_[event.type]) {
          listener->ProcessSystemEvent(event);
        }
        PushMouseUpToChildInFocus(event);
        break;

      case SystemEvent::kMouseButtonDown:
        for (auto listener : listener_table_[event.type]) {
          listener->ProcessSystemEvent(event);
        }
        PushMouseDownToChildInFocusAndTopHim(event);
        break;

      case SystemEvent::kMouseMotion:
        for (auto listener : listener_table_[event.type]) {
          listener->ProcessSystemEvent(event);
        }
        PushMouseMotionToChildInFocus(event);
        break;

      default:
        printf("ERROR: event type = %d\n", event.type);
        assert(0);
        break;
    }
  }

 private:
  std::unordered_map<SystemEvent::Type, std::unordered_set<Listener::Abstract*> > listener_table_;
};

class BasicWindow : public Container {
 public:
  BasicWindow() = delete;
  BasicWindow(const Rectangle<size_t>& position,
              std::initializer_list<Widget::Abstract*> children,
              DrawFunctor::Abstract* draw_func)
  : Container(position, children, draw_func) {}

  void ProcessSystemEvent(const SystemEvent& event) override {
    switch (event.type) {
      case SystemEvent::kMouseButtonUp:
        PushMouseUpToChildInFocus(event);
        break;

      case SystemEvent::kMouseDown:
        PushMouseDownToChildInFocusAndTopHim(event);
        break;

      case SystemEvent::kMouseMotion:
        PushMouseMotionToChildInFocus(event);
        break;
    }
  }
};

class TitleBar : public Container {
 public:
  TitleBar() = delete;
  TitleBar(const Rectangle<size_t>& position,
           std::initializer_list<Widget::Abstract*> children,
           DrawFunctor::Abstract* draw_func,
           Functor::MoveWidget* move_func,
           Widget::MainWindow* main_window)
  : Container(position, children, draw_func),
    move_func_(move_func),
    main_window_(main_window),
    listener_(nullptr) {}

  void StartDrag() {
    listener_ = new Listener::TitleBar(move_func_, this);
    main_window_->AddListener(SystemEvent::kMouseMotion, listener_);
    main_window_->AddListener(SystemEvent::kMouseUp, listener_); 
  }

  void FinishDrag() {
    main_window_->DeleteListener(SystemEvent::kMouseMotion, listener_);
    main_window_->DeleteListener(SystemEvent::kMouseUp, listener_);
    delete listener_;
    listener_ = nullptr;
  }

  void ProcessSystemEvent(const SystemEvent& event) override {
    switch (event.type) {
      case SystemEvent::kMouseButtonDown: {
        bool was_event_accepted = false;
        for (auto child : children_) {
          if (child->IsMouseCoordinatesInBound(event.info.mouse_click.coordinate)) {
            was_event_accepted = true;
            child->ProcessSystemEvent(event);
            break;
          }
        }
        if (!was_event_accepted) {
          StartDrag();
        }
        break;
      }
    }
  }

 private:
  Functor::MoveWidget* move_func_;
  Widget::MainWindow* main_window_;
  Listener::Abstract* listener_;
};

class Button : public Abstract {
 public:
  Button() = delete;
  Button(const Rectangle<size_t>& position,
         DrawFunctor::Abstract* draw_func,
         Functor::Abstract* func,
         Widget::MainWindow* main_window)
  : Abstract(position, draw_func),
    func_(func),
    main_window_(main_window),
    listener_(nullptr),
    is_pressed(false) {}

  void StartListeningMouseUp() {
    listener_ = new Listener::Button(func_, this);
    main_window_->AddListener(SystemEvent::kMouseButtonUp, listener_);
  }

  void StopListeningMouseUp() {
    main_window_->DeleteListener(SystemEvent::kMouseButtonUp, listener_);
    delete listener_;
    listener_ = nullptr;
  }

  void ProcessSystemEvent(const SystemEvent& event) override {
    switch (event.type) {
      case SystemEvent::kMouseButtonDown: {
        StartListeningMouseUp();
        break;
      }
    }
  }

 private:
  Functor::MoveWidget* func_;
  Widget::Abstract* main_window_;
  Listener::Abstract* listener_;
};