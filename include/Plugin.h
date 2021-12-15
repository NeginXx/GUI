#pragma once
#include "IPlugin.h"
#include "Widget.h"
#include "ScrollBar.h"
#include "Skins.h"

namespace Plugin {
  class Texture : public ITexture {
   public:
   	Texture(uint width, uint height, Render* render,
   		      const ::Color& color);
    Texture(const char* image_name, Render* render);

    int GetWidth() override;
    int GetHeight() override;
    Buffer ReadBuffer() override;
    void ReleaseBuffer(Buffer buffer) override;
    void LoadBuffer(Buffer buffer) override;

    void Clear(Color color) override;
    void Present() override {}
    void DrawLine(int x0, int y0, int x1, int y1, Color color) override;
    void DrawThickLine(int x0, int y0, int x1, int y1, int thickness, Color color) override;
    void DrawCircle(int x, int y, int radius, Color color) override;
    void DrawRect(int x, int y, int width, int height, Color color) override;
    void CopyTexture(ITexture* texture, int x, int y, int width, int height) override;
    void Draw(const Rectangle& position, const Point2D<int>& src = {});

   private:
    ::Texture texture_;
    Render* render_;
  };

  class TextureFactory : public ITextureFactory {
   public:
    TextureFactory() = delete;
    TextureFactory(Render* render);
    ITexture* CreateTexture(const char* filename) override;
    ITexture* CreateTexture(int32_t width, int32_t height) override;

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
    int32_t GetWidth() override;
    int32_t GetHeight() override;
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
    int32_t GetWidth() override;
    int32_t GetHeight() override;

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

    int32_t GetWidth() override;
    int32_t GetHeight() override;
    void SetText(const char* text) override;
  };

  class PreferencesPanel : public IPreferencesPanel, public Widget::Container {
   public:
    PreferencesPanel(Widget::MainWindow* main_window);

    void Attach(IWidget* widget_, int32_t x, int32_t y) override;
    int32_t GetWidth() override;
    int32_t GetHeight() override;
  };

  class WidgetFactory : public IWidgetFactory {
   public:
    WidgetFactory() = delete;
    WidgetFactory(Widget::MainWindow* main_window, Render* render);
    ~WidgetFactory() override;

    IButton* CreateDefaultButtonWithIcon(const char* icon_file_name) override;
    IButton* CreateDefaultButtonWithText(const char* text) override;
    IButton* CreateButtonWithIcon(int32_t width, int32_t height, const char* icon_file_name) override;
    IButton* CreateButtonWithText(int32_t width, int32_t height, const char* text, int32_t char_size) override;

    ISlider* CreateDefaultSlider(float range_min, float range_max) override;
    ISlider* CreateSlider(int32_t width, int32_t height, float range_min, float range_max) override;
    ISlider* CreateSlider(int32_t width, int32_t height, float thumb_width, float thumb_height, float range_min, float range_max) override;

    ILabel* CreateDefaultLabel(const char* text) override;
    ILabel* CreateLabel(int32_t width, int32_t height, const char* text, int32_t char_size) override;

    IPreferencesPanel* CreateDefaultPreferencesPanel() override;

   private:
    std::vector<::Texture*> textures_to_free_;
    std::vector<DrawFunctor::Abstract*> draw_funcs_to_free_;
    Widget::MainWindow* main_window_;
    Render* render_;
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