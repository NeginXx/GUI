#include <queue>
#include "../include/Window.h"
#include "../include/FunctorQueue.h"

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

  Canvas::Canvas(Widget::Canvas* canvas,
                 PluginTexture* painting_area,
                 Point2D<uint> coord)
  : canvas_(canvas),
    painting_area_(painting_area),
    manager_(Tool::Manager::GetInstance()),
    is_in_action_(true) {
      $;
      manager_->ActionBegin(painting_area_, Point2D<int>(coord) - canvas_->GetPosition().corner);
      $$;
    }

  void Canvas::ProcessSystemEvent(const SystemEvent& event) {
    $;
    switch (event.type) {
      case SystemEvent::kMouseButtonUp: {
        manager_->ActionEnd(painting_area_, Point2D<int>(event.info.mouse_click.coordinate));
        canvas_->FinishPainting();
        break;
      }

      case SystemEvent::kMouseMotion: {
        Point2D<int> old_mp = Point2D<int>(event.info.mouse_motion.old_mouse_pos);
        Point2D<int> new_mp = Point2D<int>(event.info.mouse_motion.new_mouse_pos);
        if (!canvas_->IsMouseCoordinatesInBound(Point2D<uint>(new_mp))) {
          if (is_in_action_) {
            manager_->ActionEnd(painting_area_, old_mp);
            is_in_action_ = false;
          }
          break;
        }

        if (!is_in_action_) {
          manager_->ActionBegin(painting_area_, new_mp);
          is_in_action_ = true;
        }

        const Point2D<int>& canvas_pos = canvas_->GetPosition().corner;
        manager_->Action(painting_area_, old_mp - canvas_pos, new_mp - old_mp);
        break;
      }

      default: assert(0);
    }
    $$;
  }

  DropdownList::DropdownList(UserWidget::DropdownList* list)
  : list_(list) {}

  void DropdownList::ProcessSystemEvent(const SystemEvent& event) {
    assert(event.type == SystemEvent::kMouseMotion);
    Point2D<uint> mc = event.info.mouse_motion.new_mouse_pos;
    if (!list_->IsMouseCoordinatesInBound(mc) &&
        !list_->button_toggler_->IsMouseCoordinatesInBound(mc)) {
      list_->StopListeningMouseMotion();
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
      (*children_.begin())->Draw();
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



  // Canvas
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  Canvas::Canvas(const Rectangle& position,
                 Widget::MainWindow* main_window,
                 Render* render)
  : Abstract(position, nullptr),
    main_window_(main_window),
    painting_listener_(nullptr)
    {
      painting_area_ = new PluginTexture(1900, 1100, render, {0, 0, 0, 0});
    }

  Canvas::~Canvas() {
    $;
    if (painting_listener_ != nullptr) {
      FinishPainting();
    }
    delete painting_area_;
    $$;
  }

  void Canvas::StartPainting(Point2D<uint> coordinate) {
    assert((int)coordinate.x >= position_.corner.x);
    assert((int)coordinate.y >= position_.corner.y);
    painting_listener_ = new Listener::Canvas(this, painting_area_, coordinate);
    main_window_->AddListener(SystemEvent::kMouseMotion, painting_listener_);
    main_window_->AddListener(SystemEvent::kMouseButtonUp, painting_listener_);
  }

  void Canvas::FinishPainting() {
    main_window_->DeleteListener(SystemEvent::kMouseMotion, painting_listener_);
    main_window_->DeleteListener(SystemEvent::kMouseButtonUp, painting_listener_);
    delete painting_listener_;
    painting_listener_ = nullptr;
  }

  void Canvas::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonDown: {
        StartPainting(event.info.mouse_click.coordinate);
        break;
      }
    }
  }

  void Canvas::Draw() {
    painting_area_->Draw(position_);
  }
}

#include "../include/GUIConstants.h"
#include "../include/Skins.h"

namespace UserWidget {
  StandardWindow::StandardWindow(const Rectangle& pos,
                                 Widget::MainWindow* main_window)
  : Container(pos, {}, kFuncDrawTexWhite)
  {
    func_close_widget_ = new Functor::CloseWidget(nullptr, nullptr);
    func_move_ = new Functor::MoveWidget(nullptr, kStandardMoveBounds);

    assert(kStandardTitlebarHeight >= kStandardButtonWidth);
    const uint button_ofs = (kStandardTitlebarHeight - kStandardButtonWidth) / 2;
    const int x = pos.corner.x;
    const int y = pos.corner.y;

    auto button_close =
    new Widget::BasicButton({{(int)pos.width - (int)kStandardButtonWidth - 2 * (int)button_ofs + x, (int)button_ofs + y}, kStandardButtonWidth , kStandardButtonWidth},
                       main_window, func_close_widget_, {kFuncDrawButtonCloseNormal, kFuncDrawButtonCloseHover});

    auto title_bar =
    new Widget::Drag({{x, y}, pos.width, kStandardTitlebarHeight}, main_window,
                     {button_close}, func_move_, kFuncDrawTexStriped);

    children_.push_front(title_bar);

    func_close_widget_->SetWidgetToClose(this);
    func_close_widget_->SetWindowParent(main_window);
    func_move_->SetWidgetToMove(this);
    main_window->AddChild(this);
  }

  StandardWindow::~StandardWindow() {
    delete func_close_widget_;
    delete func_move_;
  }

  Widget::BasicButton* PaintWindow::CreateColorButton(Render* render, uint button_width,
                                                 const Point2D<int>& coord, Widget::MainWindow* main_window) {
    Color color = {(unsigned char)(rand() % 256), (unsigned char)(rand() % 256), (unsigned char)(rand() % 256)};
    auto texture_color = new Texture(button_width, button_width, render, color);
    textures_to_free_.push_back(texture_color);
    assert(button_width > 10);
    auto func_draw_color = new DrawFunctor::ScalableTexture(texture_color, {5, 5});
    auto func_draw_color_hover = new DrawFunctor::MultipleFunctors({kFuncDrawTexMainLightExtra, func_draw_color});
    draw_funcs_to_free_.push_back(func_draw_color);
    draw_funcs_to_free_.push_back(func_draw_color_hover);

    auto func_pick_color = new Functor::PickColor(color);
    pick_color_funcs_to_free_.push_back(func_pick_color);
    return new Widget::BasicButton({coord, button_width, button_width},
                              main_window, func_pick_color, {func_draw_color, func_draw_color_hover});
  }

  Widget::BasicButton* PaintWindow::CreatePickToolButton(Plugin::ITool* tool, Render* render, uint button_width,
                                                    const Point2D<int>& coord, Widget::MainWindow* main_window) {
    static char buf[100] = {};
    sprintf(buf, "tools/%s", tool->GetIconFileName());
    auto texture = new Texture(buf, render);
    auto func_draw_texture = new DrawFunctor::ScalableTexture(texture);
    auto func_draw_hover = new DrawFunctor::MultipleFunctors({kFuncDrawTexMainLightExtra, func_draw_texture});
    auto func_set_tool = new Functor::SetTool(tool);
    auto pick_tool_button = new Widget::BasicButton({{coord.x, coord.y}, button_width, button_width},
                                               main_window, func_set_tool, {func_draw_texture, func_draw_hover});

    textures_to_free_.push_back(texture);
    draw_funcs_to_free_.push_back(func_draw_texture);
    draw_funcs_to_free_.push_back(func_draw_hover);
    set_tool_funcs_to_free_.push_back(func_set_tool);

    return pick_tool_button;
  }

  void PaintWindow::CreatePalette(Widget::Container* palette, Render* render, uint palette_width,
                                  const Point2D<int>& coord, Widget::MainWindow* main_window) {
    const uint button_width = kStandardButtonWidth + 10;
    const uint ofs = (palette_width - 2 * button_width) / 2;
    int x = coord.x + ofs;
    int cur_y = coord.y + ofs;

    ::Tool::Manager* manager = ::Tool::Manager::GetInstance();
    std::list<Plugin::ITool*>& tools = manager->GetToolsList();
    size_t i = 0;
    for (auto tool : tools) {
      int temp_x = x;
      if (i % 2 == 1) {
        temp_x += button_width;
      }
      palette->AddChild(CreatePickToolButton(tool, render, button_width,
                                             Point2D<int>{temp_x, cur_y}, main_window));
      if (i++ % 2 == 1) {
        cur_y += button_width;
      }
    }

    for (size_t i = 0; i < 10; ++i) {
      cur_y += button_width;
      palette->AddChild(CreateColorButton(render, button_width, {x, cur_y}, main_window));
      palette->AddChild(CreateColorButton(render, button_width, {x + (int)button_width, cur_y}, main_window));
    }
  }

  PaintWindow::PaintWindow(const Rectangle& pos,
                           Widget::MainWindow* main_window,
                           Render* render)
  : StandardWindow(pos, main_window)
  {
    const int x = pos.corner.x;
    const int y = pos.corner.y;
    assert(pos.width >= 50);
    const uint palette_width = 100;
    auto canvas = new Widget::Canvas({{x + (int)palette_width, y + (int)kStandardTitlebarHeight}, pos.width - palette_width, pos.height - kStandardTitlebarHeight},
                                     main_window, render);
    auto palette = new Widget::Container({{x, (int)(y + kStandardTitlebarHeight)}, palette_width, pos.height - kStandardTitlebarHeight},
                                         {}, kFuncDrawTexStripedLight);
    CreatePalette(palette, render, palette_width, {x, y + (int)kStandardTitlebarHeight}, main_window);
    AddChild(canvas);
    AddChild(palette);
  }

  PaintWindow::~PaintWindow() {
    for (auto t : textures_to_free_) delete t;
    for (auto f : draw_funcs_to_free_) delete f;
    for (auto f : pick_color_funcs_to_free_) delete f;
    for (auto f : set_tool_funcs_to_free_) delete f;
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

  DropdownList::DropdownList(const Point2D<int>& position,
                             Widget::MainWindow* main_window,
                             Widget::AbstractContainer* window_parent,
                             Widget::ButtonOnPress* button_toggler,
                             uint button_width,
                             uint button_height,
                             const std::initializer_list<ButtonInfo>& buttons,
                             const ButtonDrawInfo& button_draw_info)
  : Abstract({position, button_width, button_height * (uint)buttons.size()}, nullptr),
    main_window_(main_window),
    window_parent_(window_parent),
    button_toggler_(button_toggler),
    button_width_(button_width),
    button_height_(button_height),
    button_draw_info_(button_draw_info),
    is_visible_(false),
    hover_listener_(nullptr),
    func_(new Functor::DropdownListClose(this))
  {
    int x = position.x;
    int cur_y = position.y;
    for (auto button : buttons) {
      auto b = new BasicButtonWithText({{x, cur_y}, button_width, button_height},
                                       main_window_, button.func, button_draw_info_,
                                       button.text);
      button_list_.push_back(b);
      cur_y += button_height_;
    }
  }

  DropdownList::~DropdownList() {
    for (auto button : button_list_) {
      delete button;
    }
    if (hover_listener_ != nullptr) {
      main_window_->DeleteListener(SystemEvent::kMouseButtonUp, hover_listener_);
      delete hover_listener_;
    }
    delete func_;
  }

  void DropdownList::StartListeningMouseMotion() {
    hover_listener_ = new Listener::DropdownList(this);
    main_window_->AddListener(SystemEvent::kMouseMotion, hover_listener_);
  }

  void DropdownList::StopListeningMouseMotion() {
    main_window_->DeleteListener(SystemEvent::kMouseMotion, hover_listener_);
    delete hover_listener_;
    hover_listener_ = nullptr;
    Hide();
  }

  void DropdownList::PopUp() {
    if (!is_visible_) {
      window_parent_->AddChild(this);
      is_visible_ = true;
    }
  }

  void DropdownList::Hide() {
    if (is_visible_) {
      FunctorQueue::GetInstance().Push(func_);
    }
  }

  void DropdownList::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonDown: {
        Point2D<int> mc = static_cast<Point2D<int>>(event.info.mouse_click.coordinate);
        Hide();
        button_list_[(mc.y - position_.corner.y) / button_height_]->ProcessSystemEvent(event);
        break;
      }

      case SystemEvent::kMouseMotion: {
        Point2D<int> mc = static_cast<Point2D<int>>(event.info.mouse_motion.new_mouse_pos);
        uint idx = Min((uint)button_list_.size() - 1, (uint)(mc.y - position_.corner.y) / button_height_);
        button_list_[idx]->ProcessSystemEvent(event);
        if (hover_listener_ == nullptr) {
          StartListeningMouseMotion();
        } // else it's already processed by listener
      }
    }
  }

  Point2D<int> DropdownList::Move(const Point2D<int>& shift,
                                  const Rectangle& bounds) {
    Point2D<int> real_shift = this->Abstract::Move(shift, bounds);
    if (real_shift.x != 0 || real_shift.y == 0) {
      for (auto button : button_list_) {
        button->Move(real_shift, bounds);
      }
    }
    return real_shift;
  }

  void DropdownList::Draw() {
    for (auto button : button_list_) {
      button->Draw();
    }
  }
}