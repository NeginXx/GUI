#pragma once
#include "main.h"
#include "Render.h"
#include "Texture.h"
#include "PluginTexture.h"
#include "Tools.h"

namespace Widget {
	class Abstract;
	class AbstractContainer;
	class MainWindow;
	class Container;
	class Drag;
	class Button;
	class Canvas;
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
	 		            const Rectangle& relative_drawing_coord = {});

	 	void Action(const Rectangle& place_to_draw) override;

	 private:
	 	Texture* texture_;
	 	Rectangle relative_drawing_coord_;
	};

	class ScalableTexture : public Abstract {
	 public:
	 	ScalableTexture() = delete;
	 	ScalableTexture(Texture* texture,
	 		              const Rectangle& relative_drawing_coord = {});

	 	void Action(const Rectangle& place_to_draw) override;

	 private:
	 	Texture* texture_;
	 	Rectangle relative_drawing_coord_;
	};

	class TextTexture : public Abstract {
	 public:
	 	TextTexture() = delete;
	 	TextTexture(Texture* text,
	 		          const Rectangle& relative_drawing_coord = {});

	 	void Action(const Rectangle& place_to_draw) override;

	 private:
	 	Texture* text_;
	 	Rectangle relative_drawing_coord_;
	};

	class MultipleFunctors : public Abstract {
	 public:
	 	MultipleFunctors() = delete;
	 	MultipleFunctors(std::initializer_list<DrawFunctor::Abstract*> draw_functors_list);

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

	class OpenFile : public Abstract {
	 public:
	 	OpenFile() = delete;
	 	OpenFile(Widget::MainWindow* main_window);

	 	void SetMainWindow(Widget::MainWindow* main_window);
	  void Action() override;

	 private:
	 	Widget::MainWindow* main_window_;
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
}