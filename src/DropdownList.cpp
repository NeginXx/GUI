#include "../include/DropdownList.h"

namespace Listener {
  DropdownList::DropdownList(UserWidget::DropdownList* list)
  : list_(list) {}

  void DropdownList::ProcessSystemEvent(const SystemEvent& event) {
    Point2D<uint> mc = event.info.mouse_motion.new_mouse_pos;
    if (!list_->IsMouseCoordinatesInBound(mc) &&
        !list_->button_toggler_->IsMouseCoordinatesInBound(mc)) {
      list_->StopListeningMouseMotion();
    }
  }
}

namespace UserWidget {
  DropdownList::DropdownList(Widget::MainWindow* main_window,
                             Widget::AbstractContainer* window_parent,
                             Widget::ButtonOnPress* button_toggler,
                             uint button_width,
                             uint button_height,
                             const std::initializer_list<ButtonInfo>& buttons,
                             const ButtonDrawInfo& button_draw_info)
  : Abstract({{}, button_width, 0}, nullptr),
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
    Rectangle pos = button_toggler_->GetPosition();
    position_.corner = pos.corner + Point2D<int>{0, (int)pos.height};
    for (auto button : buttons) {
      AddButton(button);
    }
  }

  void DropdownList::AddButton(ButtonInfo button) {
    int x = position_.corner.x;
    int y = position_.corner.y + position_.height;
    auto b = new BasicButtonWithText({{x, y}, button_width_, button_height_},
                                     main_window_, button.func, button_draw_info_,
                                     button.text);
    button_list_.push_back(b);
    position_.height += button_height_;
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
      Rectangle pos = button_toggler_->GetPosition();
      Move(pos.corner + Point2D<int>{0, (int)pos.height} - position_.corner, kStandardMoveBounds);
      window_parent_->AddChild(this);
      is_visible_ = true;
    }
  }

  void DropdownList::Hide() {
    if (is_visible_) {
      FunctorQueue::GetInstance().Push(func_);
      is_visible_ = false;
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
    if (real_shift.x != 0 || real_shift.y != 0) {
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