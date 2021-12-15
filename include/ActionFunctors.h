#pragma once
#include "main.h"
#include "Texture.h"
#include "Tools.h"
#include "List.h"

namespace Widget {
  class Abstract;
  class AbstractContainer;
  class MainWindow;
  class Container;
  class Drag;
  class BasicButton;
  class ButtonOnPress;
  class Scroll;
  class Canvas;
}

enum ScrollType {
  kHorizontal,
  kVertical
};

namespace UserWidget {
	class DropdownList;
}

namespace Plugin {
  class PreferencesPanel;
}

namespace DrawFunctor {
	class Abstract {
	 public:
	 	virtual ~Abstract() = default;
	 	virtual void Action(const Rectangle& place_to_draw) = 0;
	};

	class TilingTexture : public Abstract {
	 public:
	 	TilingTexture() = delete;
	 	TilingTexture(Texture* texture,
	 		            const Point2D<uint>& offset = {});

	 	void Action(const Rectangle& place_to_draw) override;

	 protected:
	 	Texture* texture_;
	 	Point2D<uint> offset_;
	};

	class ScalableTexture : public Abstract {
	 public:
	 	ScalableTexture() = delete;
	 	ScalableTexture(Texture* texture,
	 		              const Point2D<uint>& offset = {});

	 	void Action(const Rectangle& place_to_draw) override;

	 protected:
	 	Texture* texture_;
	 	Point2D<uint> offset_;
	};

	class TextTexture : public Abstract {
	 public:
	 	TextTexture() = delete;
	 	TextTexture(Texture* text,
	 		          const Point2D<uint>& offset = {});

	 	void Action(const Rectangle& place_to_draw) override;
	 	void SetTexture(Texture* texture);

	 protected:
	 	Texture* text_;
	 	Point2D<uint> offset_;
	};

	class MultipleFunctors : public Abstract {
	 public:
	 	MultipleFunctors() = delete;
	 	MultipleFunctors(const std::initializer_list<DrawFunctor::Abstract*>& draw_functors_list);

	 	void Action(const Rectangle& place_to_draw) override;

	 protected:
	 	List<DrawFunctor::Abstract*> draw_functors_list_;
	};
}

namespace Functor {
	class Abstract {
	 public:
	 	virtual ~Abstract() = default;
	 	virtual void Action() = 0;
	};

	class MoveWidget : public Abstract {
	 public:
	 	MoveWidget() = delete;
	 	MoveWidget(Widget::Abstract* widget_to_move,
  	           const Rectangle& widget_bounds);

	 	void SetWidgetToMove(Widget::Abstract* widget_to_move_);
	 	void Action() override;
	 	void SetShift(const Point2D<int>& shift);

	 protected:
	 	Widget::Abstract* widget_to_move_;
  	Rectangle widget_bounds_;
  	Point2D<int> shift_;
	};

	class CloseWidget : public Abstract {
	 public:
	 	CloseWidget() = delete;
	 	CloseWidget(Widget::Abstract* widget_to_close,
	 		          Widget::AbstractContainer* window_parent);

	 	void SetWidgetToClose(Widget::Abstract* widget_to_close);
	 	void SetWindowParent(Widget::AbstractContainer* window_parent);
	  void Action() override;

	 protected:
	 	Widget::Abstract* widget_to_close_;
	 	Widget::AbstractContainer* window_parent_;
	};

	class OpenCanvas : public Abstract {
	 public:
	  OpenCanvas() = delete;
	  OpenCanvas(Widget::MainWindow* main_window, Render* render);
      
	  void SetMainWindow(Widget::MainWindow* main_window);
	  void Action() override;

	 protected:
	 	Widget::MainWindow* main_window_;
	 	Render* render_;
	};

	class OpenHoleWindow : public Abstract {
	 public:
	  OpenHoleWindow() = delete;
	  OpenHoleWindow(Widget::MainWindow* main_window, Render* render);

	  void Action() override;

	 protected:
	 	Widget::MainWindow* main_window_;
	 	Render* render_;
	};

	class PickColor : public Abstract {
	 public:
	 	PickColor() = delete;
	 	PickColor(const Color& color);

	  void Action() override;

	 protected:
	 	Color color_;
	};

	class DropdownListPopUp : public Abstract {
	 public:
	 	DropdownListPopUp() = delete;
	 	DropdownListPopUp(UserWidget::DropdownList* list);

	 	void SetDropdownList(UserWidget::DropdownList* list);
	  void Action() override;

	 protected:
	 	UserWidget::DropdownList* list_;
	};

	class DropdownListClose : public Abstract {
	 public:
	 	DropdownListClose() = delete;
	 	DropdownListClose(UserWidget::DropdownList* list);

	  void Action() override;

	 protected:
	 	UserWidget::DropdownList* list_;
	};

	class Scroll : public Abstract {
	 public:
	 	Scroll() = default;
	 	void SetScrollPos(float new_scroll_pos);
	 	void SetScrollType(ScrollType type);

	 protected:
	 	float scroll_pos_;
	 	ScrollType scroll_type_;
	};

	class ScrollCanvas : public Scroll {
	 public:
	 	ScrollCanvas() = delete;
	 	ScrollCanvas(Widget::Canvas* canvas);

	  void Action() override;

	 protected:
	 	Widget::Canvas* canvas_;
	};
}