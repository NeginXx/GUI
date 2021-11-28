#include "../include/Window.h"
// #include "../include/ActionCommands.h"

// Widget::Canvas* CreateStandardCanvas(Widget::MainWindow* main_window,
//                                      const Rectangle& pos,
//                                      Render* render);

namespace DrawFunctor {
	TilingTexture::TilingTexture(Texture* texture,
		                                     const Rectangle& relative_drawing_coord)
	: texture_(texture), relative_drawing_coord_(relative_drawing_coord) {}

	bool IsRelativeDrawingCoordDefault(const Rectangle& coord) {
		return coord.corner.x == 0 && coord.corner.y == 0 &&
			     coord.width == 0 && coord.height == 0;
	}

 	void TilingTexture::Action(const Rectangle& place_to_draw) {
	  Rectangle place = place_to_draw;
	  if (!IsRelativeDrawingCoordDefault(relative_drawing_coord_)) {
	  	place.corner += relative_drawing_coord_.corner;
	  	place.width = relative_drawing_coord_.width;
	  	place.height = relative_drawing_coord_.height;
	  }

		Point2D<int> max_c = Point2D<int>{place.corner.x + (int)place.width, place.corner.y + (int)place.height};
		Point2D<int> cur_c = place.corner;
		uint width = texture_->GetWidth();
		uint height = texture_->GetHeight();
		for (; cur_c.y < max_c.y; cur_c.y += height) {
			for (cur_c.x = place.corner.x; cur_c.x < max_c.x; cur_c.x += width) {
				Rectangle dst = {cur_c, Min((uint)(max_c.x - cur_c.x), width), Min((uint)(max_c.y - cur_c.y), height)};
				texture_->DrawWithNoScale(&dst);
			}
		}
	}

	ScalableTexture::ScalableTexture(Texture* texture,
		                               const Rectangle& relative_drawing_coord)
	: texture_(texture), relative_drawing_coord_(relative_drawing_coord) {}

 	void ScalableTexture::Action(const Rectangle& place_to_draw) {
 		Rectangle place = place_to_draw;
 	  if (!IsRelativeDrawingCoordDefault(relative_drawing_coord_)) {
	  	place.corner += relative_drawing_coord_.corner;
	  	place.width = relative_drawing_coord_.width;
	  	place.height = relative_drawing_coord_.height;
	  }
 		texture_->Draw(nullptr, &place);
 	}

	TextTexture::TextTexture(Texture* text,
		                       const Rectangle& relative_drawing_coord)
	: text_(text), relative_drawing_coord_(relative_drawing_coord) {}

 	void TextTexture::Action(const Rectangle& place_to_draw) {
 		Rectangle place = place_to_draw;
 	  if (!IsRelativeDrawingCoordDefault(relative_drawing_coord_)) {
	  	place.corner += relative_drawing_coord_.corner;
	  	place.width = relative_drawing_coord_.width;
	  	place.height = relative_drawing_coord_.height;
	  }
	  place.width = Min(place.width, text_->GetWidth());
	  place.height = Min(place.height, text_->GetHeight());
 		text_->Draw(nullptr, &place);
 	}

 	MultipleFunctors::MultipleFunctors(std::initializer_list<DrawFunctor::Abstract*> list)
 	: draw_functors_list_(list) {}

 	void MultipleFunctors::Action(const Rectangle& place_to_draw) {
 		for (auto func : draw_functors_list_) {
 			func->Action(place_to_draw);
 		}
 	}
}

namespace Functor {
 	MoveWidget::MoveWidget(Widget::Abstract* widget_to_move,
	                       const Rectangle& widget_bounds)
 	: widget_to_move_(widget_to_move),
 	  widget_bounds_(widget_bounds) {}

	void MoveWidget::SetWidgetToMove(Widget::Abstract* widget_to_move) {
		widget_to_move_ = widget_to_move;
	}

 	void MoveWidget::Action() {
 		widget_to_move_->Move(shift_, widget_bounds_);
 	}

 	void MoveWidget::SetShift(const Point2D<int>& shift) {
 	 shift_ = shift;
 	}

 	void CloseWidget::SetWidgetToClose(Widget::Abstract* widget_to_close) {
 		widget_to_close_ = widget_to_close;
 	}

	void CloseWidget::SetWindowParent(Widget::AbstractContainer* window_parent) {
		window_parent_ = window_parent;
	}

 	CloseWidget::CloseWidget(Widget::Abstract* widget_to_close,
 		                       Widget::AbstractContainer* window_parent)
 	: widget_to_close_(widget_to_close),
 	  window_parent_(window_parent) {}

  void CloseWidget::Action() {
    $;
  	List<Widget::Abstract*>& children = window_parent_->GetChildren();
  	bool is_deleted = false;
  	for (auto child = children.begin(); child != children.end(); ++child) {
  		if (*child == widget_to_close_) {
  			children.Pop(child);
        delete widget_to_close_;
  			is_deleted = true;
  			break;
  		}
  	}
  	if (!is_deleted) {
  		printf("Warning: Close was called but didn't close anything\n");
  	}
    $$;
  }

 	OpenFile::OpenFile(Widget::MainWindow* main_window)
 	: main_window_(main_window) {}

 	void OpenFile::SetMainWindow(Widget::MainWindow* main_window) {
 		main_window_ = main_window;
 	}

  void OpenFile::Action() {
  	printf("OpenFile action!\n");
  	// Widget::BasicWindow* window = CreateStandardWindow(main_window_, {{100, 100}, 400, 300});
  }

  OpenCanvas::OpenCanvas(Widget::MainWindow* main_window, Render* render)
 	: main_window_(main_window), render_(render) {}

 	void OpenCanvas::SetMainWindow(Widget::MainWindow* main_window) {
 		main_window_ = main_window;
 	}

  void OpenCanvas::Action() {
  	auto canvas = new UserWidget::PaintWindow({{100, 100}, 1000, 700}, main_window_, render_);
  }

 	SetTool::SetTool(Tool::Type tool)
 	: tool_(tool) {}

  void SetTool::Action() {
  	Tool::Manager& manager = Tool::Manager::GetInstance();
  	manager.SetCurTool(tool_);
  }
  
 	PickColor::PickColor(const Color& color)
 	: color_(color) {}

  void PickColor::Action() {
  	Tool::Manager& manager = Tool::Manager::GetInstance();
  	Tool::Pencil& pencil = manager.GetPencil();
  	pencil.color = color_;
  }
}