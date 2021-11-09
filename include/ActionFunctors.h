#pragma once
#include "main.h"
#include "Render.h"
#include "Texture.h"

namespace Widget {
	class Abstract;
	class Window;
	class Resize;
	class TitleBar;
	class Button;
}

namespace DrawFunctor {
	class Abstract {
	 public:
	 	virtual void Action(const Rectangle<size_t>& widget_position) = 0;
	};

	class WindowWithChunkedTexture : public Abstract {
	 public:
	 	WindowWithChunkedTexture() = delete;
	 	WindowWithChunkedTexture(Texture* texture)
	 	: texture_(texture) {};

	 	void Action(const Rectangle<size_t>& widget_postiion) override;

	 private:
	 	Texture* texture_;
	};

	class WindowWithScalableTexture : public Abstract {
	 public:
	 	WindowWithScalableTexture() = delete;
	 	WindowWithScalableTexture(Texture* texture)
	 	: texture_(texture) {};

	 	void Action(const Rectangle<size_t>& widget_postiion) override;

	 private:
	 	Texture* texture_;
	};
}

namespace Functor {
	class Abstract {
	 public:
	 	virtual void Action() = 0;
	};

	class MoveWidget {
	 public:
	 	MoveWidget() = delete;
	 	MoveWidget(Widget::Abstract* widget_to_move,
  	           const Rectangle<size_t>& widget_bounds,
  	           Point2D<int> shift)
	 	: widget_to_move_(widget_to_move),
	 	  widget_bounds_(widget_bounds),
	 	  shift_(shift) {}

	 	void Action() override {
	 		widget_to_move_->Move(shift_, widget_bounds_);
	 	}

	 	void SetShift(const Point2D<int>& shift) {
	 	 shift_ = shift;
	 	}

	 private:
	 	Widget::Abstract* widget_to_move_;
  	const Rectangle<size_t>& widget_bounds_;
  	Point2D<int> shift_;
	}

	class CloseWidget : public Abstract {
	 public:
	 	CloseWidget() = delete;
	 	CloseWidget(Widget::Abstract* widget_to_close,
	 		          Widget::Window* window_parent)
	 	: widget_to_close_(widget_to_close),
	 	  window_parent_(window_parent) {};

	  void Action() override;

	 private:
	 	Widget::Abstract* widget_to_close_;
	 	Widget::Window* window_parent_;
	};

	class OpenFile : public Abstract {
	 public:
	 	OpenFile() = delete;
	 	OpenFile(Widget::Window* parent)
	 	: parent_(parent) {}

	 	void SetParent(Widget::Window* parent) {
	 		parent_ = parent;
	 	}
	  void Action() override;

	 private:
	 	Widget::Window* parent_;
	};
}