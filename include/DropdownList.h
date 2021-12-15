#include "Widget.h"

namespace Listener {
  class DropdownList : public Abstract {
   public:
    DropdownList() = delete;
    DropdownList(UserWidget::DropdownList* list);
    ~DropdownList() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    UserWidget::DropdownList* list_;
  };
}

namespace UserWidget {
  class DropdownList : public Widget::Abstract {
   public:
    struct ButtonInfo {
      Functor::Abstract* func;
      const char* text;
    };

    DropdownList() = delete;
    DropdownList(Widget::MainWindow* main_window,
                 Widget::AbstractContainer* window_parent,
                 Widget::ButtonOnPress* button_toggler,
                 uint button_width,
                 uint button_height,
                 const std::initializer_list<ButtonInfo>& buttons,
                 const ButtonDrawInfo& button_draw_info);
    ~DropdownList();

    void AddButton(ButtonInfo button);
    void PopUp();
    void Hide();
    Point2D<int> Move(const Point2D<int>& shift,
                      const Rectangle& bounds) override;
    void Draw() override;
    void ProcessSystemEvent(const SystemEvent& event) override;

    friend Listener::DropdownList;
    friend Functor::DropdownListClose;

   private:
    Widget::MainWindow* main_window_;
    Widget::AbstractContainer* window_parent_;
    Widget::ButtonOnPress* button_toggler_;
    uint button_width_;
    uint button_height_;
    ButtonDrawInfo button_draw_info_;
    std::vector<BasicButtonWithText*> button_list_;
    bool is_visible_;
    Listener::DropdownList* hover_listener_;
    Functor::DropdownListClose* func_;

    void StartListeningMouseMotion();
    void StopListeningMouseMotion();
  };
}