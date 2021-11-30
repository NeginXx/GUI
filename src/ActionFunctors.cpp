#include "../include/Window.h"
#include "../include/GUIConstants.h"
// #include "../include/ActionCommands.h"

// Widget::Canvas* CreateStandardCanvas(Widget::MainWindow* main_window,
//                                      const Rectangle& pos,
//                                      Render* render);

namespace DrawFunctor {
	TilingTexture::TilingTexture(Texture* texture,
		                           const Point2D<uint>& offset)
	: texture_(texture), offset_(offset) {}

	bool IsOffsetDefault(const Point2D<uint>& coord) {
		return coord.x == 0 && coord.y == 0;
	}

	void ChangePlaceToDrawIfNeeded(Rectangle* place, const Point2D<uint>& offset) {
		if (!IsOffsetDefault(offset)) {
	  	place->corner += Point2D<int>(offset);
	  	assert(place->width >= 2 * offset.x);
	  	assert(place->height >= 2 * offset.y);
	  	place->width -= 2 * offset.x;
	  	place->height -= 2 * offset.y;
	  }
	}

 	void TilingTexture::Action(const Rectangle& place_to_draw) {
	  Rectangle place = place_to_draw;
	  ChangePlaceToDrawIfNeeded(&place, offset_);

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
		                               const Point2D<uint>& offset)
	: texture_(texture), offset_(offset) {}

 	void ScalableTexture::Action(const Rectangle& place_to_draw) {
 		Rectangle place = place_to_draw;
 	  ChangePlaceToDrawIfNeeded(&place, offset_);
 		texture_->Draw(nullptr, &place);
 	}

	TextTexture::TextTexture(Texture* text,
		                       const Point2D<uint>& offset)
	: text_(text), offset_(offset) {}

 	void TextTexture::Action(const Rectangle& place_to_draw) {
 		Rectangle place = place_to_draw;
 	  ChangePlaceToDrawIfNeeded(&place, offset_);
	  place.width = Min(place.width, text_->GetWidth());
	  place.height = Min(place.height, text_->GetHeight());
 		text_->Draw(nullptr, &place);
 	}

 	MultipleFunctors::MultipleFunctors(const std::initializer_list<DrawFunctor::Abstract*>& list)
 	: draw_functors_list_(list) {}

 	void MultipleFunctors::Action(const Rectangle& place_to_draw) {
 		for (auto func : draw_functors_list_) {
 			if (func != nullptr) {
 				func->Action(place_to_draw);
 			}
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
  	std::list<Widget::Abstract*>& children = window_parent_->GetChildren();
  	auto child = children.begin();
  	for (; child != children.end(); ++child) {
  		if (*child == widget_to_close_) {
  			break;
  		}
  	}
  	if (child == children.end()) {
  		printf("Warning: Close was called but didn't close anything\n");
  		return;
  	}
  	children.erase(child);
  	delete widget_to_close_;
    $$;
  }

  OpenHoleWindow::OpenHoleWindow(Widget::MainWindow* main_window, Render* render)
 	: main_window_(main_window), render_(render) {}

  void OpenHoleWindow::Action() {
  	auto hole_window = new UserWidget::HoleWindow({{0, kStandardTitlebarHeight}, 510, 700}, main_window_, render_);
  }

  OpenCanvas::OpenCanvas(Widget::MainWindow* main_window, Render* render)
 	: main_window_(main_window), render_(render) {}

 	void OpenCanvas::SetMainWindow(Widget::MainWindow* main_window) {
 		main_window_ = main_window;
 	}

  void OpenCanvas::Action() {
  	auto canvas = new UserWidget::PaintWindow({{100, 100}, 1000, 700}, main_window_, render_);
  }

 	SetTool::SetTool(Plugin::ITool* tool)
 	: tool_(tool) {}

  void SetTool::Action() {
  	Tool::Manager* manager = Tool::Manager::GetInstance();
  	manager->SetCurrentTool(tool_);
  }
  
 	PickColor::PickColor(const Color& color)
 	: color_(color) {}

  void PickColor::Action() {
  	Tool::Manager* manager = Tool::Manager::GetInstance();
  	manager->SetColor(color_);
  }

 	DropdownListPopUp::DropdownListPopUp(UserWidget::DropdownList* list)
 	: list_(list) {}

 	void DropdownListPopUp::SetDropdownList(UserWidget::DropdownList* list) {
 		list_ = list;
 	}

  void DropdownListPopUp::Action() {
  	list_->PopUp();
  }

 	DropdownListClose::DropdownListClose(UserWidget::DropdownList* list)
 	: list_(list) {}

  void DropdownListClose::Action() {
    list_->is_visible_ = false;
    std::list<Widget::Abstract*>& children = list_->window_parent_->GetChildren();
    auto it = children.begin();
    for (; it != children.end(); ++it) {
      if (*it == list_) {
        break;
      }
    }
    if (it == children.end()) {
      printf("Warning: DropdownList::Hide was called but worked inproperly\n");
      return;
    }
    children.erase(it);
    list_->button_toggler_->StopTheClick();
  }
}