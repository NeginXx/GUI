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

  ButtonClick::ButtonClick(Functor::Abstract* action_func, Widget::Button* button)
  : action_func_(action_func), button_(button) {}

  void ButtonClick::ProcessSystemEvent(const SystemEvent& event) {
    assert(event.type == SystemEvent::kMouseButtonUp);
    if (button_->IsMouseCoordinatesInBound(event.info.mouse_click.coordinate)) {
      FunctorQueue::GetInstance().Push(action_func_);
    }
    button_->StopListeningMouseUp();
  }

  ButtonHover::ButtonHover(Widget::Button* button)
  : button_(button) {}

  void ButtonHover::ProcessSystemEvent(const SystemEvent& event) {
    assert(event.type == SystemEvent::kMouseMotion);
    if (!button_->IsMouseCoordinatesInBound(event.info.mouse_motion.new_mouse_pos)) {
      button_->StopListeningMouseMotion();
    }
  }

  Canvas::Canvas(Widget::Canvas* canvas,
                 Point2D<uint> coord,
                 Texture* painting_area)
  : canvas_(canvas),
    prev_point_(coord),
    painting_area_(painting_area) {}

  void Canvas::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonUp: {
        canvas_->FinishPainting();
        break;
      }

      case SystemEvent::kMouseMotion: {
        Point2D<uint> new_mp = event.info.mouse_motion.new_mouse_pos;
        if (!canvas_->IsMouseCoordinatesInBound(new_mp)) {
          // canvas_->FinishPainting();
          break;
        }

        Point2D<int> canvas_pos = canvas_->GetPosition().corner;
        assert((int)(new_mp.x) >= canvas_pos.x);
        assert((int)(new_mp.y) >= canvas_pos.y);
        Point2D<uint> cur_point_ = { new_mp.x - canvas_pos.x,
                                     new_mp.y - canvas_pos.y };

        Tool::Manager& manager = Tool::Manager::GetInstance();
        Tool::Type tool_type = manager.GetCurTool();
        switch (tool_type) {
          case Tool::kPencil: {
            Tool::Pencil& pencil = manager.GetPencil();
            painting_area_->DrawThickLine(Point2D<int>(prev_point_), Point2D<int>(cur_point_), pencil.thickness, pencil.color);
            break;
          }

          case Tool::kEraser: {
            Tool::Eraser& eraser = manager.GetEraser();
            painting_area_->DrawThickLine(Point2D<int>(prev_point_), Point2D<int>(cur_point_), eraser.thickness, {0, 0, 0, 0});
            break;
          }

          default: assert(0);
        }

        prev_point_ = cur_point_;
        break;
      }

      default: assert(0);
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
    if (children_.GetSize() != 0) {
      for (auto it = (children_.end()).Prev(); it != children_.begin(); --it) {
        (*it)->Draw();
      }
      (*children_.begin())->Draw();
    }
  }

  List<Widget::Abstract*>& AbstractContainer::GetChildren() {
    return children_;
  }

  void AbstractContainer::AddChild(Widget::Abstract* widget) {
    children_.PushFront(widget);
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
    if (children_.GetSize() == 1) {
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
        PushMouseUpToChildInFocus(event);
        break;

      case SystemEvent::kMouseButtonDown:
        PushMouseDownToChildInFocusAndTopHim(event);
        break;

      case SystemEvent::kMouseMotion:
        PushMouseMotionToChildInFocus(event);
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




  // Button
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  // -----------------------------------------------------
  Button::~Button() {
    if (click_listener_ != nullptr) {
      main_window_->DeleteListener(SystemEvent::kMouseButtonUp, click_listener_);
      delete click_listener_;
    }
    if (hover_listener_ != nullptr) {
      main_window_->DeleteListener(SystemEvent::kMouseMotion, hover_listener_);
      delete hover_listener_;
    }
  }

  Button::Button(const Rectangle& position,
                 Widget::MainWindow* main_window,
                 Functor::Abstract* action_func,
                 const DrawFunctors& draw_funcs)
  : Abstract(position, draw_funcs.draw_func_main),
    main_window_(main_window),
    action_func_(action_func),
    draw_funcs_(draw_funcs),
    click_listener_(nullptr),
    hover_listener_(nullptr) {}

  void Button::StartListeningMouseUp() {
    click_listener_ = new Listener::ButtonClick(action_func_, this);
    main_window_->AddListener(SystemEvent::kMouseButtonUp, click_listener_);
    if (draw_funcs_.draw_func_click != nullptr) {
      SetDrawFunc(draw_funcs_.draw_func_click);
    }
  }

  void Button::StartListeningMouseMotion() {
    hover_listener_ = new Listener::ButtonHover(this);
    main_window_->AddListener(SystemEvent::kMouseMotion, hover_listener_);
    if (draw_funcs_.draw_func_hover != nullptr) {
      SetDrawFunc(draw_funcs_.draw_func_hover);
    }
  }

  void Button::StopListeningMouseUp() {
    SetDrawFunc(draw_funcs_.draw_func_main);
    main_window_->DeleteListener(SystemEvent::kMouseButtonUp, click_listener_);
    delete click_listener_;
    click_listener_ = nullptr;
  }

  void Button::StopListeningMouseMotion() {
    SetDrawFunc(draw_funcs_.draw_func_main);
    main_window_->DeleteListener(SystemEvent::kMouseMotion, hover_listener_);
    delete hover_listener_;
    hover_listener_ = nullptr;
  }

  void Button::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonDown: {
        if (action_func_ != nullptr) {
          StartListeningMouseUp();
        }
        break;
      }

      case SystemEvent::kMouseMotion: {
        if (hover_listener_ == nullptr) {
          StartListeningMouseMotion();
        } // else it's already processed by listener
        break;
      }
    }
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
      painting_area_ = new Texture(1900, 1100, render, {0, 0, 0, 0});
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
    painting_listener_ = new Listener::Canvas(this,  Point2D<uint>(Point2D<int>(coordinate) - position_.corner), painting_area_);
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
      case SystemEvent::kMouseButtonDown:
        StartPainting(event.info.mouse_click.coordinate);
        break;
    }
  }

  void Canvas::Draw() {
    painting_area_->DrawWithNoScale(&position_);
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
    new Widget::Button({{(int)pos.width - (int)kStandardButtonWidth - 2 * (int)button_ofs + x, (int)button_ofs + y}, kStandardButtonWidth , kStandardButtonWidth},
                       main_window, func_close_widget_, {kFuncDrawButtonCloseNormal, kFuncDrawButtonCloseHover});

    auto title_bar =
    new Widget::Drag({{x, y}, pos.width, kStandardTitlebarHeight}, main_window,
                     {button_close}, func_move_, kFuncDrawTexStriped);

    children_.PushFront(title_bar);

    func_close_widget_->SetWidgetToClose(this);
    func_close_widget_->SetWindowParent(main_window);
    func_move_->SetWidgetToMove(this);
    main_window->AddChild(this);
  }

  StandardWindow::~StandardWindow() {
    delete func_close_widget_;
    delete func_move_;
  }

  Widget::Button* PaintWindow::CreateColorButton(Render* render, uint button_width,
                                                 const Point2D<int>& coord, Widget::MainWindow* main_window) {
    Color color = {(unsigned char)(rand() % 256), (unsigned char)(rand() % 256), (unsigned char)(rand() % 256)};
    auto texture_color = new Texture(button_width, button_width, render, color);
    textures_to_free_.push_back(texture_color);
    assert(button_width > 10);
    auto func_draw_color = new DrawFunctor::ScalableTexture(texture_color, {{5, 5}, button_width - 10, button_width - 10});
    auto func_draw_color_hover = new DrawFunctor::MultipleFunctors({kFuncDrawTexMainLightExtra, func_draw_color});
    draw_funcs_to_free_.push_back(func_draw_color);
    draw_funcs_to_free_.push_back(func_draw_color_hover);

    auto func_pick_color = new Functor::PickColor(color);
    pick_color_funcs_to_free_.push_back(func_pick_color);
    return new Widget::Button({coord, button_width, button_width},
                              main_window, func_pick_color, {func_draw_color, func_draw_color_hover});
  }

  void PaintWindow::CreatePalette(Widget::Container* palette, Render* render, uint palette_width,
                                  const Point2D<int>& coord, Widget::MainWindow* main_window) {
    auto func_set_pencil = new Functor::SetTool(Tool::kPencil);
    auto func_set_eraser = new Functor::SetTool(Tool::kEraser);
    set_funcs_to_free_.push_back(func_set_pencil);
    set_funcs_to_free_.push_back(func_set_eraser);

    auto func_draw_pencil_hover = new DrawFunctor::MultipleFunctors({kFuncDrawTexMainLightExtra, kFuncDrawToolPencil});
    auto func_draw_eraser_hover = new DrawFunctor::MultipleFunctors({kFuncDrawTexMainLightExtra, kFuncDrawToolEraser});
    draw_funcs_to_free_.push_back(func_draw_pencil_hover);
    draw_funcs_to_free_.push_back(func_draw_eraser_hover);

    const uint button_width = kStandardButtonWidth + 10;
    const uint ofs = (palette_width - 2 * button_width) / 2;
    int x = coord.x + ofs;
    int cur_y = coord.y + ofs;
    auto pencil_button = new Widget::Button({{x, cur_y}, button_width, button_width},
                                            main_window, func_set_pencil, {kFuncDrawToolPencil, func_draw_pencil_hover});
    auto eraser_button = new Widget::Button({{x + (int)button_width, cur_y}, button_width, button_width},
                                            main_window, func_set_eraser, {kFuncDrawToolEraser, func_draw_eraser_hover});
    cur_y += button_width;
    palette->AddChild(pencil_button);
    palette->AddChild(eraser_button);

    for (size_t i = 0; i < 10; ++i) {
      palette->AddChild(CreateColorButton(render, button_width, {x, cur_y}, main_window));
      palette->AddChild(CreateColorButton(render, button_width, {x + (int)button_width, cur_y}, main_window));
      cur_y += button_width;
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
    for (auto f : draw_funcs_to_free_) delete f;
    for (auto f : set_funcs_to_free_) delete f;
    for (auto f : pick_color_funcs_to_free_) delete f;
    for (auto t : textures_to_free_) delete t;
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

  ButtonWithText::ButtonWithText(const Rectangle& position,
                                 Widget::MainWindow* main_window,
                                 Functor::Abstract* action_func,
                                 const DrawFunctors& draw_funcs,
                                 const char* text,
                                 Render* render,
                                 const Color& color)
  : Button(position, main_window, action_func, {}),
    text_(new Texture(text, render, color)),
    draw_text_(new DrawFunctor::TextTexture(text_))
  {
    const DrawFunctors& df = draw_funcs;
    DrawFunctors& df_ = draw_funcs_;
    #define INIT_DF(type) \
      if (df.draw_func_##type != nullptr) { \
        df_.draw_func_##type = new DrawFunctor::MultipleFunctors({df.draw_func_##type, draw_text_}); \
      }
      INIT_DF(main)
      INIT_DF(hover)
      INIT_DF(click)
    #undef INIT_DF
    SetDrawFunc(df_.draw_func_main);
  }

  ButtonWithText::ButtonWithText(const Point2D<int>& position,
                                 Widget::MainWindow* main_window,
                                 Functor::Abstract* action_func,
                                 const DrawFunctors& draw_funcs,
                                 const char* text,
                                 Render* render,
                                 const Color& color)
  : ButtonWithText({{0, 0}, 0, 0}, main_window, action_func, draw_funcs, text, render, color)
  {
    position_ = {position, text_->GetWidth(), text_->GetHeight()};
  }

  ButtonWithText::~ButtonWithText() {
    delete text_;
    delete draw_text_;
    DrawFunctors& df = draw_funcs_;
    #define DELETE_DF(type) \
      if (df.draw_func_##type != nullptr) { \
        delete df.draw_func_##type; \
      }
      DELETE_DF(main)
      DELETE_DF(hover)
      DELETE_DF(click)
    #undef DELETE_DF
  }
}