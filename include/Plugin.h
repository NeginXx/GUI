#pragma once
#include "IPlugin.h"
#include "Widget.h"
#include "ScrollBar.h"
#include "Skins.h"

namespace Plugin {
  class WidgetFactory;
  class Icon;

  class Texture : public ITexture {
   public:
   	Texture(uint width, uint height, Render* render,
   		      const ::Color& color);
    Texture(const char* image_name, Render* render);

    uint GetWidth() override;
    uint GetHeight() override;

    Buffer ReadBuffer() override;
    void ReleaseBuffer(Buffer buffer) override;
    void LoadBuffer(Buffer buffer) override;

    void Clear(Color Color) override;
    void Present() override {}

    void DrawLine  (const Line& line) override;
    void DrawCircle(const Circle& circle) override;
    void DrawRect  (const Rect& rect) override;

    void CopyTexture(ITexture* source, int x, int y, uint width, uint height) override;
    void CopyTexture(ITexture* source, int x, int y) override;
    void Draw(const Rectangle& position, const Point2D<int>& src = {});
    friend class Icon;

   private:
    ::Texture texture_;
    Render* render_;
  };

  class TextureFactory : public ITextureFactory {
   public:
    TextureFactory() = delete;
    TextureFactory(Render* render);
    ITexture* CreateTexture(const char* filename) override;
    ITexture* CreateTexture(uint width, uint height) override;

   private:
    Render* render_;
  };

  class Button : public IButton, public Widget::BasicButton {
    class Functor : public ::Functor::Abstract {
     public:
      Functor() = delete;
      Functor(IClickCallback* callback);

      void Action() override;
      void SetCallback(IClickCallback* callback);

     private:
      IClickCallback* callback_;
    };

   public:
    Button(const Rectangle& position,
           Widget::MainWindow* main_window,
           const Widget::ButtonDrawFunctors& draw_funcs);
    ~Button() override;
    uint GetWidth() override;
    uint GetHeight() override;
    void SetClickCallback(IClickCallback* callback) override;

   private:
    Functor* func_;
  };

  class Slider : public ISlider, public Widget::Scroll {
    class Functor : public ::Functor::Scroll {
     public:
      Functor() = delete;
      Functor(ISliderCallback* callback,
              float range_min, float range_max);

      void Action() override;
      void SetCallback(ISliderCallback* callback);

     private:
      ISliderCallback* callback_;
      float old_val_;
      float range_min_;
      float range_max_;
    };

   public:
    Slider(const Rectangle& position,
           Widget::MainWindow* main_window,
           int bound0,
           int bound1,
           const Widget::ButtonDrawFunctors& draw_funcs,
           float range_min,
           float range_max);
    ~Slider() override;

    void SetSliderCallback(ISliderCallback* callback) override;
    float GetValue() override;
    void SetValue(float value) override;
    uint GetWidth() override;
    uint GetHeight() override;

   private:
    float range_min_;
    float range_max_;
    Functor* func_;
  };

  class Label : public ILabel, public UserWidget::Label {
   public:
    Label() = delete;
    Label(const Point2D<int>& position,
          const char* text,
          Render* render,
          const ::Color& color);

    uint GetWidth() override;
    uint GetHeight() override;
    void SetText(const char* text) override;
  };

  class Icon : public IIcon, public Widget::Icon {
   public:
    Icon() = delete;
    Icon(Rectangle position);
    ~Icon() override;

    uint GetWidth() override;
    uint GetHeight() override;
    void SetIcon(const ITexture* icon) override;
  };

  struct Palette : public IPalette {
    void SetPaletteCallback(IPaletteCallback* callback) override {callback_ = callback;}
    uint GetWidth() override {return 0;}
    uint GetHeight() override {return 0;}
    IPaletteCallback* callback_;
  };

  class PreferencesPanel : public IPreferencesPanel, public Widget::Container {
   public:
    PreferencesPanel(Widget::MainWindow* main_window,
                     WidgetFactory* widget_factory);
    uint GetWidth() override;
    uint GetHeight() override;

    void Attach(IButton*  button,  int x, int y) override;
    void Attach(ILabel*   label,   int x, int y) override;
    void Attach(ISlider*  slider,  int x, int y) override;
    void Attach(IIcon*    icon,    int x, int y) override;
    void Attach(IPalette* palette, int x, int y) override;
    void Attach(IWidget* widget,   int x, int y);

   private:
    WidgetFactory* widget_factory_;
  };

  class WidgetFactory : public IWidgetFactory {
   public:
    WidgetFactory() = delete;
    WidgetFactory(Widget::MainWindow* main_window, Render* render);
    ~WidgetFactory() override;

    IButton* CreateDefaultButtonWithIcon(const char* icon_file_name) override;
    IButton* CreateDefaultButtonWithText(const char* text) override;
    IButton* CreateButtonWithIcon(uint width, uint height, const char* icon_file_name) override;
    IButton* CreateButtonWithText(uint width, uint height, const char* text, uint char_size) override;

    ISlider* CreateDefaultSlider(float range_min, float range_max) override;
    ISlider* CreateSlider(uint width, uint height, float range_min, float range_max) override;
    // ISlider* CreateSlider(uint width, uint height, float thumb_width, float thumb_height, float range_min, float range_max) override;

    ILabel* CreateDefaultLabel(const char* text) override;
    ILabel* CreateLabel(uint width, uint height, const char* text, uint char_size) override;

    IIcon* CreateIcon(uint width, uint height) override;
    IPalette* CreatePalette() override;
    IPreferencesPanel* CreatePreferencesPanel() override;

    std::vector<Palette*> GetPalettes() {return palettes_;}
    void AddPalette(Palette* p) {palettes_.push_back(p);}

   private:
    std::vector<::Texture*> textures_to_free_;
    std::vector<DrawFunctor::Abstract*> draw_funcs_to_free_;
    Widget::MainWindow* main_window_;
    Render* render_;
    std::vector<Palette*> palettes_;
  };

  class API : public IAPI {
   public:
    API() = delete;
    API(Widget::MainWindow* main_window, Render* render);

    IWidgetFactory* GetWidgetFactory() override;
    ITextureFactory* GetTextureFactory() override;

   private:
    WidgetFactory w_factory_;
    TextureFactory t_factory_;
  };
}