#include <queue>
#include <iostream>
#include "../include/Widget.h"
#include "../include/FunctorQueue.h"
#include "../include/Skins.h"

namespace Listener {
  Drag::Drag(Functor::MoveWidget* move_func, Widget::Drag* widget_drag)
  : move_func_(move_func), widget_drag_(widget_drag) {}

  void Drag::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonUp: {
        widget_drag_->FinishDrag();
        break;
      }

      case SystemEvent::kMouseMotion: {
        auto info = event.info.mouse_motion;
        int x_diff = (int)info.new_mouse_pos.x - (int)info.old_mouse_pos.x;
        int y_diff = (int)info.new_mouse_pos.y - (int)info.old_mouse_pos.y;
        move_func_->SetShift(Point2D<int>{x_diff, y_diff});
        FunctorQueue::GetInstance().Push(move_func_);
        break;
      }

      default: assert(0);
    }
  }

  BasicButtonClick::BasicButtonClick(Functor::Abstract* action_func, Widget::BasicButton* button)
  : action_func_(action_func), button_(button) {}

  void BasicButtonClick::ProcessSystemEvent(const SystemEvent& event) {
    assert(event.type == SystemEvent::kMouseButtonUp);
    if (button_->IsMouseCoordinatesInBound(event.info.mouse_click.coordinate)) {
      FunctorQueue::GetInstance().Push(action_func_);
    }
    button_->StopListeningMouseUp();
  }

  BasicButtonHover::BasicButtonHover(Widget::BasicButton* button)
  : button_(button) {}

  void BasicButtonHover::ProcessSystemEvent(const SystemEvent& event) {
    assert(event.type == SystemEvent::kMouseMotion);
    if (!button_->IsMouseCoordinatesInBound(event.info.mouse_motion.new_mouse_pos)) {
      button_->StopListeningMouseMotion();
    }
  }

  ButtonOnPress::ButtonOnPress(Widget::ButtonOnPress* button)
  : button_(button) {}

  void ButtonOnPress::ProcessSystemEvent(const SystemEvent& event) {
    assert(event.type == SystemEvent::kMouseMotion);
    if (!button_->IsMouseCoordinatesInBound(event.info.mouse_motion.new_mouse_pos)) {
      button_->StopListeningMouseMotion();
    }
  }
}

namespace Widget {
  // Abstract
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------

  Abstract::Abstract(const Rectangle& position,
                     DrawFunctor::Abstract* draw_func)
  : position_(position), draw_func_(draw_func) {}

  Rectangle Abstract::GetPosition() {
    return position_;
  }

  void Abstract::SetPosition(const Rectangle& pos) {
    position_ = pos;
  }

  void Abstract::SetDrawFunc(DrawFunctor::Abstract* draw_func) {
    draw_func_ = draw_func;
  }

  Point2D<int> Abstract::Move(const Point2D<int>& shift,
                              const Rectangle& bounds) {
    Point2D<int> real_shift = {};
    Point2D<int> min_coord = bounds.corner;
    Point2D<int> max_coord = bounds.corner + Point2D<int>{ (int)(bounds.width),
                                                           (int)(bounds.height) };

    if (shift.x >= 0) {
      uint bound_dist = (uint)(Max(0, max_coord.x - position_.corner.x - (int)(position_.width)));
      uint temp = Min(bound_dist, (uint)(shift.x));
      position_.corner.x += temp;
      real_shift.x = temp;
    } else {
      uint bound_dist = (uint)(Max(0, position_.corner.x - min_coord.x));
      uint shift_positive = (uint)(-shift.x);
      uint temp = Min(bound_dist, shift_positive);
      position_.corner.x -= temp;
      real_shift.x = -temp;
    }

    if (shift.y >= 0) {
      uint bound_dist = (uint)(Max(0, max_coord.y - position_.corner.y - (int)position_.height));
      uint temp = Min(bound_dist, (uint)(shift.y));
      position_.corner.y += temp;
      real_shift.y = temp;
    } else {
      uint bound_dist = (uint)(Max(0, position_.corner.y - min_coord.y));
      uint shift_positive = (uint)(-shift.y);
      uint temp = Min(bound_dist, shift_positive);
      position_.corner.y -= temp;
      real_shift.y = -temp;
    }

    return real_shift;
  }

  void Abstract::Resize(const Point2D<int>& corner_shift,
                        int width_shift, int height_shift,
                        const Rectangle& bounds) {
    if (corner_shift.x >= 0) {
      if (width_shift >= 0) {
        // size_t bound_dist = (size_t)Max(0, (int)max_coord.x - (int)(position_.x + position_.width));
        position_.width += width_shift;
      } else {
        position_.corner.x += corner_shift.x;
      }
    }
  }

  bool Abstract::IsMouseCoordinatesInBound(const Point2D<uint>& mouse_coord) {
    const Point2D<uint>& m_c = mouse_coord;
    Rectangle pos = position_;
    return pos.corner.x <= (int)m_c.x && (int)m_c.x <= pos.corner.x + (int)pos.width &&
           pos.corner.y <= (int)m_c.y && (int)m_c.y <= pos.corner.y + (int)pos.height;
  }

  void Abstract::Draw() {
    if (draw_func_ != nullptr) {
      draw_func_->Action(position_);
    }
  }




  // AbstractContainer
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------

  AbstractContainer::AbstractContainer(const Rectangle& position,
                                       std::initializer_list<Widget::Abstract*> children,
                                       DrawFunctor::Abstract* draw_func)
  : Abstract(position, draw_func), children_(children) {}

  AbstractContainer::~AbstractContainer() {
    $;
    DeleteChildren();
    $$;
  }

  void AbstractContainer::DeleteChildren() {
    $;
    for (auto child : children_) {
      delete child;
    }
    $$;
  }

  void AbstractContainer::DrawChildren() {
    if (children_.size() != 0) {
      for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        (*it)->Draw();
      }
    }
  }

  std::list<Widget::Abstract*>& AbstractContainer::GetChildren() {
    return children_;
  }

  void AbstractContainer::AddChild(Widget::Abstract* widget) {
    children_.push_front(widget);
  }

  #define PUSH_EVENT(event_info)          \
    for (auto child : children_) {        \
      if (child->IsMouseCoordinatesInBound(event.info.event_info)) { \
        child->ProcessSystemEvent(event); \
        break;                            \
      }                                   \
    } 

  void AbstractContainer::PushMouseUpToChildInFocus(const SystemEvent& event) {
    PUSH_EVENT(mouse_click.coordinate);
  }

  void AbstractContainer::PushMouseMotionToChildInFocus(const SystemEvent& event) {
    PUSH_EVENT(mouse_motion.new_mouse_pos);
  }


  void AbstractContainer::PushMouseDownToChildInFocusAndTopHim(const SystemEvent& event) {
    if (children_.size() == 1) {
      PUSH_EVENT(mouse_click.coordinate);
      return;
    }

    for (auto child_it = children_.begin(); child_it != children_.end(); ++child_it) {
      if ((*child_it)->IsMouseCoordinatesInBound(event.info.mouse_click.coordinate)) {
        Widget::Abstract* child = *child_it;
        child->ProcessSystemEvent(event);
        if (child_it != children_.begin()) {
          children_.erase(child_it);
          children_.push_front(child);
        }
        break;
      }
    }
  }

  #undef PUSH_EVENT

  void AbstractContainer::Draw() {
    this->Abstract::Draw();
    DrawChildren();
  }

  Point2D<int> AbstractContainer::Move(const Point2D<int>& shift,
                                       const Rectangle& bounds) {
    Point2D<int> real_shift = this->Abstract::Move(shift, bounds);
    if (!(real_shift.x == 0 && real_shift.y == 0)) {
      for (auto child : children_) {
        child->Move(real_shift, bounds);
      }
    }

    return real_shift;
  }

  // MainWindow
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  MainWindow::MainWindow(const Rectangle& position,
                         std::initializer_list<Widget::Abstract*> children,
                         DrawFunctor::Abstract* draw_func)
  : AbstractContainer(position, children, draw_func) {
    listener_table_[SystemEvent::kMouseButtonUp];
    listener_table_[SystemEvent::kMouseButtonDown];
    listener_table_[SystemEvent::kMouseMotion];
  }

  void MainWindow::AddListener(SystemEvent::Type event_type, Listener::Abstract* listener) {
    auto it = listener_table_.find(event_type);
    assert(it != listener_table_.end());
    listener_table_[event_type].insert(listener);
  }

  void MainWindow::DeleteListener(SystemEvent::Type event_type, Listener::Abstract* listener) {
    auto it = listener_table_.find(event_type);
    assert(it != listener_table_.end());
    auto& set = listener_table_[event_type];
    size_t sz = listener_table_[event_type].size();
    if (set.find(listener) == set.end()) {
      assert(0);
    } else {
      set.erase(listener);
      assert(listener_table_[event_type].size() == sz - 1);
    }
  }

  bool MainWindow::IsListenerProcessed(Listener::Abstract* listener) {
    return processed_listeners_.find(listener) != processed_listeners_.end();
  }

  void MainWindow::SendEventToListeners(const SystemEvent& event) {
    $;
    assert(listener_table_.find(event.type) != listener_table_.end());
    bool is_end = false;
    while (!is_end) {
      auto& listeners = listener_table_[event.type];
      auto it = listeners.begin();

      for (; it != listeners.end(); ++it) {
        if (!IsListenerProcessed(*it)) {
          processed_listeners_.insert(*it);
          (*it)->ProcessSystemEvent(event);
          break;
        }
      }

      if (it == listeners.end()) {
        is_end = true;
      }
    }

    processed_listeners_.clear();
    $$;
  }

  void MainWindow::ProcessSystemEvent(const SystemEvent& event) {
    $;
    switch (event.type) {
      case SystemEvent::kMouseButtonUp:
        SendEventToListeners(event);
        PushMouseUpToChildInFocus(event);
        break;

      case SystemEvent::kMouseButtonDown:
        SendEventToListeners(event);
        PushMouseDownToChildInFocusAndTopHim(event);
        break;

      case SystemEvent::kMouseMotion:
        SendEventToListeners(event);
        PushMouseMotionToChildInFocus(event);
        break;

      case SystemEvent::kWindowResize:
        break;

      default:
        printf("ERROR: event type = %d\n", event.type);
        assert(0);
        break;
    }
    $$;
  }




  // Container
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  Container::Container(const Rectangle& position,
                           std::initializer_list<Widget::Abstract*> children,
                           DrawFunctor::Abstract* draw_func)
  : AbstractContainer(position, children, draw_func) {}

  void Container::ProcessSystemEvent(const SystemEvent& event) {
    $;
    switch (event.type) {
      case SystemEvent::kMouseButtonUp:
        $;
        PushMouseUpToChildInFocus(event);
        $$;
        break;

      case SystemEvent::kMouseButtonDown:
        $;
        PushMouseDownToChildInFocusAndTopHim(event);
        $$;
        break;

      case SystemEvent::kMouseMotion:
        $;
        PushMouseMotionToChildInFocus(event);
        $$;
        break;
    }
    $$;
  }




  // Drag
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  Drag::~Drag() {
    if (drag_listener_ != nullptr) {
      FinishDrag();
    }
  }

  Drag::Drag(const Rectangle& position,
             Widget::MainWindow* main_window,
             std::initializer_list<Widget::Abstract*> children,
             Functor::MoveWidget* move_func,
             DrawFunctor::Abstract* draw_func)
  : AbstractContainer(position, children, draw_func),
    main_window_(main_window),
    move_func_(move_func),
    drag_listener_(nullptr) {}

  void Drag::StartDrag() {
    drag_listener_ = new Listener::Drag(move_func_, this);
    main_window_->AddListener(SystemEvent::kMouseMotion, drag_listener_);
    main_window_->AddListener(SystemEvent::kMouseButtonUp, drag_listener_);
  }

  void Drag::FinishDrag() {
    main_window_->DeleteListener(SystemEvent::kMouseMotion, drag_listener_);
    main_window_->DeleteListener(SystemEvent::kMouseButtonUp, drag_listener_);
    delete drag_listener_;
    drag_listener_ = nullptr;
  }

  void Drag::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonUp: {
        PushMouseUpToChildInFocus(event);
        break;
      }

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

      case SystemEvent::kMouseMotion: {
        if (drag_listener_ == nullptr) {
          PushMouseMotionToChildInFocus(event);
        }
        break;
      }
    }
  }




  // BasicButton
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  BasicButton::~BasicButton() {
    if (click_listener_ != nullptr) {
      main_window_->DeleteListener(SystemEvent::kMouseButtonUp, click_listener_);
      delete click_listener_;
    }
    if (hover_listener_ != nullptr) {
      main_window_->DeleteListener(SystemEvent::kMouseMotion, hover_listener_);
      delete hover_listener_;
    }
  }

  BasicButton::BasicButton(const Rectangle& position,
                 Widget::MainWindow* main_window,
                 Functor::Abstract* action_func,
                 const ButtonDrawFunctors& draw_funcs)
  : Abstract(position, draw_funcs.draw_func_main),
    main_window_(main_window),
    action_func_(action_func),
    draw_funcs_(draw_funcs),
    click_listener_(nullptr),
    hover_listener_(nullptr) {}

  ButtonDrawFunctors BasicButton::GetDrawFuncs() {
    return draw_funcs_;
  }

  void BasicButton::StartListeningMouseUp() {
    click_listener_ = new Listener::BasicButtonClick(action_func_, this);
    main_window_->AddListener(SystemEvent::kMouseButtonUp, click_listener_);
    if (draw_funcs_.draw_func_click != nullptr) {
      draw_func_ = draw_funcs_.draw_func_click;
    }
  }

  void BasicButton::StartListeningMouseMotion() {
    hover_listener_ = new Listener::BasicButtonHover(this);
    main_window_->AddListener(SystemEvent::kMouseMotion, hover_listener_);
    if (draw_funcs_.draw_func_hover != nullptr) {
      draw_func_ = draw_funcs_.draw_func_hover;
    }
  }

  void BasicButton::StopListeningMouseUp() {
    main_window_->DeleteListener(SystemEvent::kMouseButtonUp, click_listener_);
    delete click_listener_;
    click_listener_ = nullptr;
    if (draw_func_ == draw_funcs_.draw_func_click) {
      draw_func_ = draw_funcs_.draw_func_main;
    }
  }

  void BasicButton::StopListeningMouseMotion() {
    main_window_->DeleteListener(SystemEvent::kMouseMotion, hover_listener_);
    delete hover_listener_;
    hover_listener_ = nullptr;
    if (draw_func_ == draw_funcs_.draw_func_hover) {
      draw_func_ = draw_funcs_.draw_func_main;
    }
  }

  void BasicButton::Action() {
    FunctorQueue::GetInstance().Push(action_func_);
  }

  void BasicButton::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonDown: {
        if (action_func_ != nullptr) {
          StartListeningMouseUp();
        }
        break;
      }

      case SystemEvent::kMouseMotion: {
        if (hover_listener_ == nullptr) {
          if (click_listener_ == nullptr) {
           StartListeningMouseMotion();
          }
        } // else it's already processed by listener
        break;
      }
    }
  }




  // ButtonOnPress
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  ButtonOnPress::ButtonOnPress(const Rectangle& position,
                               Widget::MainWindow* main_window,
                               Functor::Abstract* action_func,
                               const ButtonDrawFunctors& draw_funcs)
  : Abstract(position, draw_funcs.draw_func_main),
    main_window_(main_window),
    action_func_(action_func),
    draw_funcs_(draw_funcs),
    hover_listener_(nullptr),
    is_in_click_state_(false) {}

  ButtonOnPress::~ButtonOnPress() {
    if (hover_listener_ != nullptr) {
      main_window_->DeleteListener(SystemEvent::kMouseButtonUp, hover_listener_);
      delete hover_listener_;
    }
  }

  void ButtonOnPress::StartListeningMouseMotion() {
    hover_listener_ = new Listener::ButtonOnPress(this);
    main_window_->AddListener(SystemEvent::kMouseMotion, hover_listener_);
    if (draw_funcs_.draw_func_hover != nullptr) {
      draw_func_ = draw_funcs_.draw_func_hover;
    }
  }

  void ButtonOnPress::StopListeningMouseMotion() {
    main_window_->DeleteListener(SystemEvent::kMouseMotion, hover_listener_);
    delete hover_listener_;
    hover_listener_ = nullptr;
    if (draw_func_ == draw_funcs_.draw_func_hover) {
      draw_func_ = draw_funcs_.draw_func_main;
    }
  }

  void ButtonOnPress::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonDown: {
        is_in_click_state_ = true;
        if (action_func_ != nullptr) {
          if (draw_funcs_.draw_func_click != nullptr) {
            draw_func_ = draw_funcs_.draw_func_click;
          }
          FunctorQueue::GetInstance().Push(action_func_);
        }
        break;
      }

      case SystemEvent::kMouseMotion: {
        if (hover_listener_ == nullptr) {
          if (!is_in_click_state_) {
           StartListeningMouseMotion();
          } // else it's in a click state and no need to do that
        } // else it's already processed by listener
        break;
      }
    }
  }

  void ButtonOnPress::StopTheClick() {
    draw_func_ = draw_funcs_.draw_func_main;
    is_in_click_state_ = false;
  }
}

namespace UserWidget {
  StandardWindow::StandardWindow(const Rectangle& pos,
                                 Widget::MainWindow* main_window)
  : Container({pos.corner - Point2D<int>{(int)kStandardResizeOfs, (int)kStandardResizeOfs},
              pos.width + 2 * kStandardResizeOfs, pos.height + 2 * kStandardResizeOfs}, {}, kFuncDrawTexBlack)
  {
    AddChild(new Container(pos, {}, kFuncDrawTexMain));
    func_close_widget_ = new Functor::CloseWidget(nullptr, nullptr);
    func_move_ = new Functor::MoveWidget(nullptr, kStandardMoveBounds);

    assert(kStandardTitlebarHeight >= kStandardButtonWidth);
    const uint button_ofs = (kStandardTitlebarHeight - kStandardButtonWidth) / 2;
    const int x = pos.corner.x;
    const int y = pos.corner.y;

    button_close_ =
    new Widget::BasicButton({{(int)pos.width - (int)kStandardButtonWidth - 2 * (int)button_ofs + x, (int)button_ofs + y}, kStandardButtonWidth , kStandardButtonWidth},
                            main_window, func_close_widget_, {kFuncDrawButtonCloseNormal, kFuncDrawButtonCloseHover});

    auto title_bar =
    new Widget::Drag({{x, y}, pos.width, kStandardTitlebarHeight}, main_window,
                     {button_close_}, func_move_, kFuncDrawTexMain);

    AddChild(title_bar);

    func_close_widget_->SetWidgetToClose(this);
    func_close_widget_->SetWindowParent(main_window);
    func_move_->SetWidgetToMove(this);
    main_window->AddChild(this);
  }

  StandardWindow::~StandardWindow() {
    delete func_close_widget_;
    delete func_move_;
  }

  void StandardWindow::Close() {
    button_close_->Action();
  }

  HoleWindow::HoleWindow(const Rectangle& position,
                         Widget::MainWindow* main_window,
                         Render* render)
  : Drag(position, main_window, {},
         new Functor::MoveWidget(this, kStandardMoveBounds),
         nullptr),
    texture_(new Texture(position_.width, position_.height, render, {0, 0, 0, 0}))
  {
    for (int y = 0; y < (int)position_.height; ++y) {
      for (int x = 0; x < (int)position_.width; ++x) {
        if (IsInBound(Point2D{x, y} + position_.corner)) {
          texture_->DrawPoint({x, y}, {213, 121, 121});
        }
      }
    }
    main_window->AddChild(this);
  }

  HoleWindow::~HoleWindow() {
    delete texture_;
    delete move_func_;
  }

  bool HoleWindow::IsInBound(const Point2D<int>& mouse_coord) {
    uint radius = Min(position_.width / 5, position_.height / 5);
    Point2D<int> center = position_.corner + Point2D<int>{(int)position_.width, (int)position_.height} / 2;
    Point2D<int> coord_relative = center - mouse_coord;
    return pow(2.0 / (double)position_.width * (double)coord_relative.x, 10) +
           pow(2.0 / (double)position_.height * (double)coord_relative.y, 10) < 1.0
           && pow(coord_relative.x, 2) + pow(coord_relative.y, 2) > pow(radius, 2);
  }

  bool HoleWindow::IsMouseCoordinatesInBound(const Point2D<uint>& mouse_coordinates) {
    return IsInBound(Point2D<int>(mouse_coordinates));
  }

  void HoleWindow::Draw() {
    texture_->Draw(nullptr, &position_);
  }

  BasicButtonWithText::BasicButtonWithText(const Rectangle& position,
                                           Widget::MainWindow* main_window,
                                           Functor::Abstract* action_func,
                                           const ButtonDrawInfo& button_draw_info,
                                           const char* text)
  : BasicButton(position, main_window, action_func, {}),
    text_(new Texture(text, button_draw_info.render, button_draw_info.text_color)),
    draw_text_(new DrawFunctor::TextTexture(text_, {kTextWidthOfs, kTextHeightOfs}))
  {
    const Widget::ButtonDrawFunctors& df = button_draw_info.draw_funcs;
    Widget::ButtonDrawFunctors& df_ = draw_funcs_;
    #define INIT_DF(type) \
      if (df.draw_func_##type != nullptr) { \
        df_.draw_func_##type = new DrawFunctor::MultipleFunctors({df.draw_func_##type, draw_text_}); \
      }
      INIT_DF(main)
      INIT_DF(hover)
      INIT_DF(click)
    #undef INIT_DF
    draw_func_ = df_.draw_func_main;
  }

  BasicButtonWithText::BasicButtonWithText(const Point2D<int>& position,
                                           Widget::MainWindow* main_window,
                                           Functor::Abstract* action_func,
                                           const ButtonDrawInfo& button_draw_info,
                                           const char* text)
  : BasicButtonWithText({position, 0, kStandardButtonHeight}, main_window, action_func, button_draw_info, text)
  {
    position_.width = text_->GetWidth() + 2 * kTextWidthOfs;
  }

  BasicButtonWithText::~BasicButtonWithText() {
    delete text_;
    delete draw_text_;
    Widget::ButtonDrawFunctors& df = draw_funcs_;
    #define DELETE_DF(type) \
      if (df.draw_func_##type != nullptr) { \
        delete df.draw_func_##type; \
      }
      DELETE_DF(main)
      DELETE_DF(hover)
      DELETE_DF(click)
    #undef DELETE_DF
  }

  ButtonOnPressWithText::ButtonOnPressWithText(const Point2D<int>& position,
                                               Widget::MainWindow* main_window,
                                               Functor::Abstract* action_func,
                                               const ButtonDrawInfo& button_draw_info,
                                               const char* text)
  : ButtonOnPress({position, 0, kStandardButtonHeight}, main_window, action_func, {}),
    text_(new Texture(text, button_draw_info.render, button_draw_info.text_color)),
    draw_text_(new DrawFunctor::TextTexture(text_, {kTextWidthOfs, kTextHeightOfs}))
  {
    position_.width = text_->GetWidth() + 2 * kTextWidthOfs;
    const Widget::ButtonDrawFunctors& df = button_draw_info.draw_funcs;
    Widget::ButtonDrawFunctors& df_ = draw_funcs_;
    #define INIT_DF(type) \
      if (df.draw_func_##type != nullptr) { \
        df_.draw_func_##type = new DrawFunctor::MultipleFunctors({df.draw_func_##type, draw_text_}); \
      }
      INIT_DF(main)
      INIT_DF(hover)
      INIT_DF(click)
    #undef INIT_DF
    draw_func_ = df_.draw_func_main;
  }

  ButtonOnPressWithText::~ButtonOnPressWithText() {
    delete text_;
    delete draw_text_;
    Widget::ButtonDrawFunctors& df = draw_funcs_;
    #define DELETE_DF(type) \
      if (df.draw_func_##type != nullptr) { \
        delete df.draw_func_##type; \
      }
      DELETE_DF(main)
      DELETE_DF(hover)
      DELETE_DF(click)
    #undef DELETE_DF
  }

  Label::Label(const Point2D<int>& position,
               const char* text,
               Render* render,
               const Color& color)
  : Abstract({position, 0, kFontHeight + 2 * kTextHeightOfs}, nullptr),
    render_(render),
    text_(new Texture(text, render_, color)),
    draw_text_(new DrawFunctor::TextTexture(text_, {kTextWidthOfs, kTextHeightOfs}))
  {
    position_.width = text_->GetWidth() + 2 * kTextWidthOfs;
    draw_func_ = draw_text_;
  }

  void Label::SetText(const char* text) {
    delete text_;
    text_ = new ::Texture(text, render_, kWhite);
    position_.width = text_->GetWidth() + 2 * kTextWidthOfs;
    draw_text_->SetTexture(text_);
  }

  Label::~Label() {
    delete text_;
    delete draw_text_;
  }
}