#include <iostream>
#include "../include/Canvas.h"
#include "../include/Skins.h"
#include "../include/GUIConstants.h"
#include "../include/ScrollBar.h"
#include "../include/DropdownList.h"

const uint kPaletteWidth = 200 - kStandardResizeOfs;

class MainBar : public Widget::Container {
 public:
  MainBar() = delete;
  ~MainBar() override;
  MainBar(Widget::MainWindow* main_window,
          Render* render, uint width);
 private:
  Functor::OpenCanvas* func_open_canvas_;
  Functor::OpenHoleWindow* func_open_hole_window_;
  Functor::DropdownListPopUp* func_;
  UserWidget::DropdownList* dropdown_list_;
};
extern MainBar* kMainBar;

namespace DrawFunctor {
  PaletteButtonHighlight::PaletteButtonHighlight
  (uint width, Point2D<uint> ofs, Render* render, Color color)
  : TilingTexture(new Texture(width, width, render, {0, 0, 0, 0}), ofs) {
    for (uint y = 0; y < width; ++y) {
      for (uint x = 0; x < width; ++x) {
        double x_double = x;
        double y_double = y;
        double half_width = (double)width / 2.0;
        if (pow(1.0 / half_width * (x_double - half_width), 10) +
            pow(1.0 / half_width * (y_double - half_width), 10) <= 1.0) {
          texture_->DrawPoint({(int)x, (int)y}, color);
        }
      }
    }
  }

  PaletteButtonHighlight::~PaletteButtonHighlight() {
    delete texture_;
  }
}

namespace Listener {
  Point2D<int> Canvas::CalculateRelativeCoordinate(const Point2D<uint>& mouse_coordinates) {
    return Point2D<int>(mouse_coordinates) - canvas_->GetPosition().corner + Point2D<int>(canvas_->GetViewPos());
  }

  Canvas::Canvas(Widget::Canvas* canvas,
                 Plugin::Texture* painting_area,
                 Point2D<uint> mouse_coord)
  : canvas_(canvas),
    painting_area_(painting_area),
    manager_(Tool::Manager::GetInstance()),
    is_in_action_(true),
    prev_coord_(mouse_coord)
  {
    manager_->ActionBegin(painting_area_, CalculateRelativeCoordinate(mouse_coord));
  }

  void Canvas::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonUp: {
        manager_->ActionEnd(painting_area_, Point2D<int>(event.info.mouse_click.coordinate) - canvas_->GetPosition().corner + Point2D<int>(canvas_->GetViewPos()));
        canvas_->FinishPainting();
        break;
      }

      case SystemEvent::kMouseMotion: {
        Point2D<uint> new_mp = event.info.mouse_motion.new_mouse_pos;
        if (!canvas_->IsMouseCoordinatesInBound(new_mp)) {
          if (is_in_action_) {
            manager_->ActionEnd(painting_area_, CalculateRelativeCoordinate(prev_coord_));
            is_in_action_ = false;
          }
          break;
        }

        if (!is_in_action_) {
          manager_->ActionBegin(painting_area_, CalculateRelativeCoordinate(new_mp));
          is_in_action_ = true;
        }

        manager_->Action(painting_area_, CalculateRelativeCoordinate(prev_coord_), Point2D<int>(new_mp) - Point2D<int>(prev_coord_));
        prev_coord_ = new_mp;
        break;
      }

      default: assert(0);
    }
  }
}

namespace Widget {
  Canvas::Canvas(const Rectangle& position,
                 Widget::MainWindow* main_window,
                 Render* render)
  : Abstract(position, nullptr),
    main_window_(main_window),
    area_width_(3000),
    area_height_(2000),
    painting_area_(new Plugin::Texture(area_width_, area_height_, render, {0, 0, 0, 0})),
    painting_listener_(nullptr),
    view_pos_(0, 0) {}

  Canvas::~Canvas() {
    $;
    if (painting_listener_ != nullptr) {
      FinishPainting();
    }
    delete painting_area_;
    $$;
  }

  void Canvas::StartPainting(Point2D<uint> mouse_coordinate) {
    assert((int)mouse_coordinate.x >= position_.corner.x);
    assert((int)mouse_coordinate.y >= position_.corner.y);
    painting_listener_ = new Listener::Canvas(this, painting_area_, mouse_coordinate);
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
      case SystemEvent::kMouseButtonDown: {
        StartPainting(event.info.mouse_click.coordinate);
        break;
      }
    }
  }

  void Canvas::Draw() {
    kTextureTexWhite->DrawWithNoScale(&position_);
    painting_area_->Draw(position_, Point2D<int>(view_pos_));
  }

  void Canvas::ChangeViewPos(float new_scroll_pos,
                             ScrollType scroll_type_) {
    assert(new_scroll_pos >= 0.0f);
    assert(new_scroll_pos <= 1.0f);
    if (scroll_type_ == ScrollType::kHorizontal) {
      assert(area_width_ >= position_.width);
      view_pos_.x = (uint)Max(0, (int)((float)(area_width_ - position_.width) * new_scroll_pos));
    }else if (scroll_type_ == kVertical) {
      assert(area_height_ >= position_.height);
      view_pos_.y = (uint)Max(0, (int)((float)(area_height_ - position_.height) * new_scroll_pos));
    }
  }

  Point2D<uint> Canvas::GetViewPos() {
    return view_pos_;
  }
}

namespace Functor {
  SetTool::SetTool(UserWidget::PaintWindow* paint_window,
                   Plugin::ITool* tool,
                   Widget::MainWindow* main_window,
                   Widget::BasicButton* tool_button)
  : paint_window_(paint_window), tool_(tool),
    main_window_(main_window),
    tool_button_(tool_button) {}

  void SetTool::SetToolButton(Widget::BasicButton* tool_button) {
    tool_button_ = tool_button;
  }

  void SetTool::Action() {
    if (tool_button_ != nullptr) {
      auto draw_funcs = tool_button_->GetDrawFuncs();
      tool_button_->SetDrawFunc(draw_funcs.draw_func_click);
    }
    paint_window_->TogglePrefPanel(tool_);
    Tool::Manager* manager = Tool::Manager::GetInstance();
    manager->SetCurrentTool(tool_);
  }

  ApplyFilter::ApplyFilter(Plugin::IFilter* filter,
                           Plugin::ITexture* canvas)
  : filter_(filter), canvas_(canvas) {}

  void ApplyFilter::Action() {
    filter_->Apply(canvas_);
  }
}

namespace UserWidget {
  Widget::BasicButton* PaintWindow::CreateColorButton(Render* render, uint button_width,
                                                 const Point2D<int>& coord, Widget::MainWindow* main_window,
                                                 UserWidget::DropdownList* list) {
    unsigned char red = rand() % 256;
    unsigned char green = rand() % 256;
    unsigned char blue = rand() % 256;
    Color color = {red, green, blue};
    auto func_pick_color = new Functor::PickColor(color);
    char buf[100] = {};
    sprintf(buf, "Color %u %u %u", red, green, blue);
    list->AddButton({func_pick_color, buf});

    auto texture_color = new Texture(button_width, button_width, render, color);
    textures_to_free_.push_back(texture_color);
    assert(button_width > 10);
    auto func_draw_color = new DrawFunctor::ScalableTexture(texture_color, {5, 5});
    auto func_draw_color_hover = new DrawFunctor::MultipleFunctors({kFuncDrawTexMainLightExtra, func_draw_color});
    draw_funcs_to_free_.push_back(func_draw_color);
    draw_funcs_to_free_.push_back(func_draw_color_hover);

    pick_color_funcs_to_free_.push_back(func_pick_color);
    return new Widget::BasicButton({coord, button_width, button_width},
                                   main_window, func_pick_color, {func_draw_color, func_draw_color_hover});
  }

  Widget::BasicButton* PaintWindow::CreatePickToolButton(Plugin::ITool* tool, Render* render, uint button_width,
                                                         const Point2D<int>& coord, Widget::MainWindow* main_window) {
    static char buf[100] = {};
    sprintf(buf, "tools/%s", tool->GetIconFileName());
    auto texture = new Texture(buf, render);
    auto func_draw_texture = new DrawFunctor::ScalableTexture(texture, {5, 5});

    auto func_draw_highlight0 = new DrawFunctor::PaletteButtonHighlight(button_width - 2, {1, 1}, render, {50, 50, 50});
    auto func_draw_highlight1 = new DrawFunctor::PaletteButtonHighlight(button_width - 2, {1, 1}, render, {30, 30, 30});
    auto func_draw_highlight_frame = new DrawFunctor::PaletteButtonHighlight(button_width, {0, 0}, render, kBlack);
    auto func_draw_hover = new DrawFunctor::MultipleFunctors({func_draw_highlight_frame, func_draw_highlight0, func_draw_texture});
    auto func_draw_click = new DrawFunctor::MultipleFunctors({func_draw_highlight_frame, func_draw_highlight1, func_draw_texture});

    auto func_set_tool = new Functor::SetTool(this, tool, main_window, nullptr);
    auto pick_tool_button = new Widget::BasicButton({{coord.x, coord.y}, button_width, button_width},
                                                    main_window, func_set_tool, {func_draw_texture, func_draw_hover, func_draw_click});
    func_set_tool->SetToolButton(pick_tool_button);

    textures_to_free_.push_back(texture);
    draw_funcs_to_free_.push_back(func_draw_texture);
    draw_funcs_to_free_.push_back(func_draw_highlight0);
    draw_funcs_to_free_.push_back(func_draw_highlight1);
    draw_funcs_to_free_.push_back(func_draw_highlight_frame);
    draw_funcs_to_free_.push_back(func_draw_hover);
    draw_funcs_to_free_.push_back(func_draw_click);
    set_tool_funcs_to_free_.push_back(func_set_tool);

    return pick_tool_button;
  }

  void PaintWindow::CreatePalette(Widget::Container* palette, Render* render,
                                  const Point2D<int>& coord, Widget::MainWindow* main_window) {
    const uint button_width = kStandardButtonWidth + 10;
    const uint ofs = 15;
    const uint buttons_in_row = (kPaletteWidth - 2 * ofs) / button_width;
    const uint actual_ofs = (kPaletteWidth - buttons_in_row * button_width) / 2;
    int x = coord.x + actual_ofs;
    int cur_y = coord.y + actual_ofs;

    ::Tool::Manager* manager = ::Tool::Manager::GetInstance();
    std::list<Plugin::ITool*>& tools = manager->GetToolsList();

    auto func = new Functor::DropdownListPopUp(nullptr);
    auto some_button =
    new UserWidget::ButtonOnPressWithText({41, 0}, main_window, func, {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra},
                                          render, kWhite}, "Tools");
    auto tools_list =
    new UserWidget::DropdownList(main_window, main_window,
                                 some_button, 200, kStandardTitlebarHeight, {},
                                 {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra}, render, kWhite});
    func->SetDropdownList(tools_list);
    kMainBar->AddChild(some_button);
    int _width = some_button->GetPosition().width;



    auto func0 = new Functor::DropdownListPopUp(nullptr);
    auto button_colors =
    new UserWidget::ButtonOnPressWithText({41 + _width, 0}, main_window, func0, {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra},
                                          render, kWhite}, "Colors");
    auto colors_list =
    new UserWidget::DropdownList(main_window, main_window,
                                 button_colors, 200, kStandardTitlebarHeight, {},
                                 {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra}, render, kWhite});
    func0->SetDropdownList(colors_list);
    kMainBar->AddChild(button_colors);

    uint i = 0;
    for (auto tool : tools) {
      tools_list->AddButton({new Functor::SetTool(this, tool, main_window, nullptr), tool->GetName()});
      pref_panels_[tool] = tool->GetPreferencesPanel();
      int temp_x = x + button_width * (i % buttons_in_row);
      palette->AddChild(CreatePickToolButton(tool, render, button_width,
                                             Point2D<int>{temp_x, cur_y}, main_window));
      if (i % buttons_in_row == buttons_in_row - 1) {
        cur_y += button_width;
      }
      ++i;
    }

    for (uint i = 0; i < 20; ++i) {
      palette->AddChild(CreateColorButton(render, button_width, {x + (int)(button_width * (i % buttons_in_row)), cur_y}, main_window, colors_list));
      if (i % buttons_in_row == buttons_in_row - 1) {
        cur_y += button_width;
      }
    }
  }

  void PaintWindow::TogglePrefPanel(Plugin::ITool* tool) {
    bool was_deleted = false;
    if (cur_pref_panel_ != nullptr) {
      for (auto it = children_.begin(); it != children_.end(); ++it) {
        if (*it == cur_pref_panel_) {
          children_.erase(it);
          was_deleted = true;
          break;
        }
      }
    }
    assert(was_deleted || cur_pref_panel_ == nullptr);
    Plugin::IPreferencesPanel* _panel = pref_panels_[tool];
    if (_panel == nullptr) {
      cur_pref_panel_ = nullptr;
      return;
    }
    Widget::AbstractContainer* panel = dynamic_cast<Widget::AbstractContainer*>(_panel);
    cur_pref_panel_ = panel;
    Rectangle pos = panel->GetPosition();
    panel->Move(position_.corner + Point2D<int>{kStandardResizeOfs, (int)(position_.height - pos.height - kStandardResizeOfs)} - pos.corner, kStandardMoveBounds);
    AddChild(panel);
  }

  PaintWindow::PaintWindow(const Rectangle& pos,
                           Widget::MainWindow* main_window,
                           Render* render)
  : StandardWindow(pos, main_window),
    cur_pref_panel_(nullptr)
  {
    const int x = pos.corner.x;
    const int y = pos.corner.y;
    assert(pos.width >= 50);

    // Creating canvas
    // -------------------------------------------------
    Rectangle canvas_back_pos = {{x + (int)kPaletteWidth, y + (int)kStandardTitlebarHeight}, pos.width - kPaletteWidth, pos.height - kStandardTitlebarHeight};
    Rectangle canvas_pos = {canvas_back_pos.corner + Point2D<int>{(int)kStandardResizeOfs, (int)kStandardResizeOfs}, canvas_back_pos.width - kStandardResizeOfs, canvas_back_pos.height - kStandardResizeOfs};
    auto canvas = new Widget::Canvas(canvas_pos, main_window, render);
    AddChild(new Container(canvas_back_pos, {}, kFuncDrawTexBlack));
    AddChild(canvas);

    // Creating dropdownlist for filters
    // -------------------------------------------------
    ::Tool::Manager* manager = ::Tool::Manager::GetInstance();
    std::list<Plugin::IFilter*> filters = manager->GetFiltersList();

    auto func = new Functor::DropdownListPopUp(nullptr);
    auto filters_button =
    new UserWidget::ButtonOnPressWithText({x, y}, main_window, func, {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra},
                                          render, kWhite}, "Filters");

    auto filters_list =
    new UserWidget::DropdownList(main_window, this,
                                 filters_button, 200, kStandardTitlebarHeight, {},
                                 {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra}, render, kWhite});
    func->SetDropdownList(filters_list);
    for (auto filter : filters) {
      filters_list->AddButton({new Functor::ApplyFilter(filter, canvas->GetPaintingArea()), filter->GetName()});
    }
    AddChild(filters_button);

    // Creating palette
    // -------------------------------------------------
    Rectangle palette_back_pos = {{x, (int)(y + kStandardTitlebarHeight)}, kPaletteWidth, pos.height - kStandardTitlebarHeight};
    Rectangle palette_pos = {palette_back_pos.corner + Point2D<int>{0, (int)kStandardResizeOfs}, palette_back_pos.width - kStandardResizeOfs, palette_back_pos.height - kStandardResizeOfs};
    auto palette = new Widget::Container(palette_pos,
                                         {}, kFuncDrawTexMain);
    CreatePalette(palette, render, {x, y + (int)kStandardTitlebarHeight}, main_window);
    AddChild(new Container(palette_back_pos, {}, kFuncDrawTexBlack));
    AddChild(palette);

    // Creating sliders
    // -------------------------------------------------
    const uint scroll_bar_width = 84;
    const int scroll_ofs = 2;
    scroll_canvas0_ = new Functor::ScrollCanvas(canvas);
    scroll_canvas1_ = new Functor::ScrollCanvas(canvas);
    int c_x = canvas_pos.corner.x;
    int c_y = canvas_pos.corner.y;
    int c_w = canvas_pos.width;
    int c_h = canvas_pos.height;
    AddChild(new Widget::Scroll({{c_x + scroll_ofs, c_y + c_h - (int)kStandardThumbWidth - scroll_ofs}, scroll_bar_width, kStandardThumbWidth},
                                main_window, kHorizontal, c_x + scroll_ofs, c_x + c_w - scroll_ofs, scroll_canvas0_, {kFuncDrawHorizontalScrollBarNormal, kFuncDrawHorizontalScrollBarHover, kFuncDrawHorizontalScrollBarClick}));

    AddChild(new Widget::Scroll({{c_x + c_w - (int)kStandardThumbWidth - scroll_ofs, c_y + scroll_ofs}, kStandardThumbWidth, scroll_bar_width},
                                main_window, kVertical, c_y + scroll_ofs, c_y + c_h - scroll_ofs, scroll_canvas1_, {kFuncDrawVerticalScrollBarNormal, kFuncDrawVerticalScrollBarHover, kFuncDrawVerticalScrollBarClick}));
  }

  void PaintWindow::ProcessSystemEvent(const SystemEvent& event) {
    switch (event.type) {
      case SystemEvent::kMouseButtonUp:
        PushMouseUpToChildInFocus(event);
        break;

      case SystemEvent::kMouseButtonDown:
        for (auto child : children_) {
          if (child->IsMouseCoordinatesInBound(event.info.mouse_click.coordinate)) {
            child->ProcessSystemEvent(event);
            break;
          }
        }
        break;

      case SystemEvent::kMouseMotion:
        PushMouseMotionToChildInFocus(event);
        break;
    }
  }

  PaintWindow::~PaintWindow() {
    for (auto t : textures_to_free_) delete t;
    for (auto f : draw_funcs_to_free_) delete f;
    for (auto f : pick_color_funcs_to_free_) delete f;
    for (auto f : set_tool_funcs_to_free_) delete f;
    delete scroll_canvas0_;
    delete scroll_canvas1_;
  }
}