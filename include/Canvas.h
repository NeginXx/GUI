#include "Widget.h"
#include "Plugin.h"

namespace DrawFunctor {
  class PaletteButtonHighlight : public TilingTexture {
   public:
    PaletteButtonHighlight() = delete;
    PaletteButtonHighlight(uint width,
                           Point2D<uint> ofs,
                           Render* render,
                           Color color);
    ~PaletteButtonHighlight() override;
  };
}

namespace Listener {
  class Canvas : public Abstract {
   public:
    Canvas() = delete;
    Canvas(Widget::Canvas* canvas,
           Plugin::Texture* painting_area,
           Point2D<uint> mouse_coord);
    ~Canvas() override = default;

    void ProcessSystemEvent(const SystemEvent& event) override;

   private:
    Widget::Canvas* canvas_;
    Plugin::Texture* painting_area_;
    Tool::Manager* manager_;
    bool is_in_action_;
    Point2D<uint> prev_coord_;

    Point2D<int> CalculateRelativeCoordinate(const Point2D<uint>& mouse_coordinates);
  };
}

namespace Widget {
  class Canvas : public Abstract {
   public:
    Canvas() = delete;
    Canvas(const Rectangle& position,
           Widget::MainWindow* main_window,
           Render* render);
    ~Canvas() override;

    void ProcessSystemEvent(const SystemEvent& event) override;
    void Draw() override;
    void ChangeViewPos(float new_scroll_pos,
                       ScrollType scroll_type_);
    Plugin::Texture* GetPaintingArea() {
      return painting_area_;
    }
    Point2D<uint> GetViewPos();
    friend Listener::Canvas;

   protected:
    Widget::MainWindow* main_window_;
    uint area_width_;
    uint area_height_;
    Plugin::Texture* painting_area_;
    Listener::Canvas* painting_listener_;
    Point2D<uint> view_pos_;

    void StartPainting(Point2D<uint> mouse_coordinate);
    void FinishPainting();
  };
}

namespace UserWidget {
  class PaintWindow;
}

namespace Functor {
  class SetTool : public Abstract {
   public:
    SetTool() = delete;
    SetTool(UserWidget::PaintWindow* paint_window,
            Plugin::ITool* tool,
            Widget::MainWindow* main_window,
            Widget::BasicButton* tool_button);

    void SetToolButton(Widget::BasicButton* tool_button);
    void Action() override;

   protected:
    UserWidget::PaintWindow* paint_window_;
    Plugin::ITool* tool_;
    Widget::MainWindow* main_window_;
    Widget::BasicButton* tool_button_;
  };

  class ApplyFilter : public Abstract {
   public:
    ApplyFilter() = delete;
    ApplyFilter(Plugin::IFilter* filter,
                Plugin::ITexture* canvas);

    // void SetToolButton(Widget::BasicButton* tool_button);
    void Action() override;

   protected:
    Plugin::IFilter* filter_;
    Plugin::ITexture* canvas_;
  };
}

namespace UserWidget {
  class PaintWindow : public StandardWindow {
   public:
    PaintWindow() = delete;
    PaintWindow(const Rectangle& pos,
                Widget::MainWindow* main_window,
                Render* render);
    ~PaintWindow() override;

    void ProcessSystemEvent(const SystemEvent& event) override;
    void TogglePrefPanel(Plugin::ITool* tool);

   private:
    std::unordered_map<Plugin::ITool*, Plugin::IPreferencesPanel*> pref_panels_;
    Widget::AbstractContainer* cur_pref_panel_;
    std::vector<Texture*> textures_to_free_;
    std::vector<DrawFunctor::Abstract*> draw_funcs_to_free_;
    std::vector<Functor::PickColor*> pick_color_funcs_to_free_;
    std::vector<Functor::SetTool*> set_tool_funcs_to_free_;
    Functor::ScrollCanvas* scroll_canvas0_;
    Functor::ScrollCanvas* scroll_canvas1_;

    void CreatePalette(Widget::Container* palette, Render* render,
                       const Point2D<int>& coord, Widget::MainWindow* main_window);
    Widget::BasicButton* CreateColorButton(Render* render, uint button_width,
                                           const Point2D<int>& coord, Widget::MainWindow* main_window, UserWidget::DropdownList* list);
    Widget::BasicButton* CreatePickToolButton(Plugin::ITool* tool, Render* render, uint button_width,
                                              const Point2D<int>& coord, Widget::MainWindow* main_window);
  };
}