#include "../include/Window.h"
// #include "../include/ActionCommands.h"

extern Widget::Window* CreateStandardWindow(Widget::Window* parent,
	                                          const Rectangle<size_t>& pos);

namespace DrawFunctor {
 	void WindowWithChunkedTexture::Draw(const Rectangle<size_t>& widget_position) {
		const Rectangle<size_t>& w_p = widget_position;
		Point2D<size_t> max_c = Point2D<size_t>{w_p.corner.x + w_p.width, w_p.corner.y + w_p.height};
		Point2D<size_t> cur_c = w_p.corner;
		size_t width = texture_->GetWidth();
		size_t height = texture_->GetHeight();
		for (; cur_c.y < max_c.y; cur_c.y += height) {
			for (cur_c.x = w_p.corner.x; cur_c.x < max_c.x; cur_c.x += width) {
				Rectangle<size_t> dst = {cur_c, Min(max_c.x - cur_c.x, width), Min(max_c.y - cur_c.y, height)};
				texture_->DrawFittablePart(&dst);
			}
		}
	}

 	void WindowWithScalableTexture::Draw(const Rectangle<size_t>& widget_position) {
 		texture_->Draw(nullptr, &widget_position);
 	}
}

namespace ResizeFunctor {
 	void WindowBasic::Resize(int x_diff, int y_diff) {
 		// printf("Resize!\n");
 		// window_->Resize(x_diff, y_diff);
 		// List<Widget::Abstract*>& children = window_->GetChildren();
 		// for (auto child : children) {
 		// 	child->Resize(x_diff, y_diff);
 		// }
 	}
}

namespace MoveFunctor {
	void TitleBar::SetWidgetToMove(Widget::Abstract* widget_to_move) {
		widget_ = widget_to_move;
	}

 	void TitleBar::Move(int x_diff, int y_diff) {
 		widget_->Move(x_diff, y_diff);
 	}
}

namespace ActionFunctor {
  void CloseButton::Action() {
  	List<Widget::Abstract*>& children = window_parent_->GetChildren();
  	bool is_deleted = false;
  	for (auto child = children.begin(); child != children.end(); ++child) {
  		if (*child == widget_to_close_) {
  			children.Pop(child);
  			is_deleted = true;
  			break;
  		}
  	}
  	if (!is_deleted) {
  		printf("Warning: Close was called but didn't close anyone\n");
  	}
  }

  void FileButton::Action() {
    Widget::Window* new_window = CreateStandardWindow(parent_, {{500, 500}, 800, 500});
  }
}

void CanvasFunctor::Action(const Point2D<size_t>& p1,
						               const Point2D<size_t>& p2) {
	if (cur_instrument_ == kPencil) {
  	texture_->DrawLine(p1, p2);
	} else if (cur_instrument_ == kEraser) {
  	texture_->DrawLine(p1, p2, {255, 255, 255, 0});
	} else {
		assert(0);
	}
}

void CanvasFunctor::Draw(const Rectangle<size_t>& position) {
	Rectangle<size_t> src = {{0, 0}, position.width, position.height};
	texture_->Draw(&src, &position);
}

void CanvasFunctor::SetCanvasInstrument(CanvasInstrument instrument) {
	cur_instrument_ = instrument;
}