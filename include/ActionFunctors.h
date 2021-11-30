#pragma once
#include "main.h"
#include "Window.h"
#include "Texture.h"
#include "PluginTexture.h"
#include "Tools.h"

namespace Widget {
  class Abstract;
  class AbstractContainer;
  class MainWindow;
  class Container;
  class Drag;
  class BasicButton;
  class ButtonOnPress;
  class Canvas;
  class DropdownList;
}

namespace UserWidget {
	class DropdownList;
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

	 private:
	 	Texture* texture_;
	 	Point2D<uint> offset_;
	};

	class ScalableTexture : public Abstract {
	 public:
	 	ScalableTexture() = delete;
	 	ScalableTexture(Texture* texture,
	 		              const Point2D<uint>& offset = {});

	 	void Action(const Rectangle& place_to_draw) override;

	 private:
	 	Texture* texture_;
	 	Point2D<uint> offset_;
	};

	class TextTexture : public Abstract {
	 public:
	 	TextTexture() = delete;
	 	TextTexture(Texture* text,
	 		          const Point2D<uint>& offset = {});

	 	void Action(const Rectangle& place_to_draw) override;

	 private:
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

	 private:
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

	 private:
	 	Widget::Abstract* widget_to_close_;
	 	Widget::AbstractContainer* window_parent_;
	};

	class OpenCanvas : public Abstract {
	 public:
	  OpenCanvas() = delete;
	  OpenCanvas(Widget::MainWindow* main_window, Render* render);
      
	  void SetMainWindow(Widget::MainWindow* main_window);
	  void Action() override;

	 private:
	 	Widget::MainWindow* main_window_;
	 	Render* render_;
	};

	class OpenHoleWindow : public Abstract {
	 public:
	  OpenHoleWindow() = delete;
	  OpenHoleWindow(Widget::MainWindow* main_window, Render* render);

	  void Action() override;

	 private:
	 	Widget::MainWindow* main_window_;
	 	Render* render_;
	};

	class SetTool : public Abstract {
	 public:
	 	SetTool() = delete;
	 	SetTool(Plugin::ITool* tool);

	  void Action() override;

	 private:
	 	Plugin::ITool* tool_;
	};

	class PickColor : public Abstract {
	 public:
	 	PickColor() = delete;
	 	PickColor(const Color& color);

	  void Action() override;

	 private:
	 	Color color_;
	};

	class DropdownListPopUp : public Abstract {
	 public:
	 	DropdownListPopUp() = delete;
	 	DropdownListPopUp(UserWidget::DropdownList* list);

	 	void SetDropdownList(UserWidget::DropdownList* list);
	  void Action() override;

	 private:
	 	UserWidget::DropdownList* list_;
	};

	class DropdownListClose : public Abstract {
	 public:
	 	DropdownListClose() = delete;
	 	DropdownListClose(UserWidget::DropdownList* list);

	  void Action() override;

	 private:
	 	UserWidget::DropdownList* list_;
	};
}