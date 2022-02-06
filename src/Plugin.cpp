#include <SDL2/SDL.h>
#include "../include/Plugin.h"
#include "../include/Render.h"

const uint kPrefPanelWidth = 200 - 2 * kStandardResizeOfs;
const uint kPrefPanelHeight = 200;
const uint kPrefPanelOfs = 10;
const uint kDefaultButtonWidth = kStandardButtonWidth;

namespace Plugin {
  Texture::Texture(uint width, uint height, Render* render,
  	               const ::Color& color)
  : texture_(width, height, render, color), render_(render) {}

  Texture::Texture(const char* image_name, Render* render)
  : texture_(image_name, render) {}

  uint Texture::GetWidth() {
  	return texture_.GetWidth();
  }

  uint Texture::GetHeight() {
  	return texture_.GetHeight();
  }

  Buffer Texture::ReadBuffer() {
    Color* buffer = new Color[GetWidth() * GetHeight()];
    SDL_SetRenderTarget(render_->render_, texture_.texture_);
    assert(!SDL_RenderReadPixels(render_->render_, NULL, SDL_PIXELFORMAT_RGBA8888,
                                 buffer, GetWidth() * sizeof(Color)));
    // for (uint y = 0; y < GetHeight(); ++y) {
    //   for (uint x = 0; x < GetWidth(); ++x) {
    //     uint p = buffer[y * GetHeight() + x];
    //     buffer[y * GetHeight() + x] = InvertttColor(p);
    //   }
    // }
    return {buffer, this};
  }

  void Texture::ReleaseBuffer(Buffer buffer) {
    delete[] buffer.pixels;
  }

  void Texture::LoadBuffer(Buffer buffer) {
    // for (uint y = 0; y < GetHeight(); ++y) {
    //   for (uint x = 0; x < GetWidth(); ++x) {
    //     uint p = buffer.pixels[y * GetHeight() + x];
    //     buffer.pixels[y * GetHeight() + x] = InvertColor(p);
    //   }
    // }
    assert(!SDL_UpdateTexture(texture_.texture_, NULL, buffer.pixels, GetWidth() * sizeof(Color)));
  }

  void Texture::Clear(Color color) {
    texture_.SetBackgroundColor(GetColor(color));
  }

  void Texture::DrawLine(const Line& line) {
    texture_.DrawThickLine(Point2D<int>{line.x0, line.y0},
                           Point2D<int>{line.x1, line.y1},
                           line.thickness,
                           GetColor(line.color));
  }

  void Texture::DrawCircle(const Circle& circle) {
    texture_.DrawCircle(Point2D<int>{circle.x, circle.y}, circle.radius, GetColor(circle.fill_color));
  }

  void Texture::DrawRect(const Rect& rect) {
    texture_.DrawRect({{rect.x, rect.y}, rect.width, rect.height}, GetColor(rect.fill_color));
  }

  void Texture::CopyTexture(ITexture* source, int x, int y, uint width, uint height) {
    Rectangle dst = {{x, y}, width, height};
    texture_.CopyTexture(dynamic_cast<Texture*>(source)->texture_, &dst);
  }

  void Texture::CopyTexture(ITexture* source, int x, int y) {
    CopyTexture(source, x, y, source->GetWidth(), source->GetHeight());
  }

  void Texture::Draw(const Rectangle& position, const Point2D<int>& src) {
    texture_.DrawWithNoScale(&position, src);
  }

  TextureFactory::TextureFactory(Render* render)
  : render_(render) {}

  ITexture* TextureFactory::CreateTexture(const char* filename) {
    return new Texture(filename, render_);
  }

  ITexture* TextureFactory::CreateTexture(uint width, uint height) {
    return new Texture(width, height, render_, {0, 0, 0, 0});
  }

  Button::Functor::Functor(IClickCallback* callback)
  : callback_(callback) {}

  void Button::Functor::Action() {
    if (callback_ != nullptr) {
      callback_->RespondOnClick();
    }
  }

  void Button::Functor::SetCallback(IClickCallback* callback) {
    callback_ = callback;
  }

  uint Button::GetWidth() {
    return position_.width;
  }

  uint Button::GetHeight() {
    return position_.height;
  }

  Button::Button(const Rectangle& position,
                 Widget::MainWindow* main_window,
                 const Widget::ButtonDrawFunctors& draw_funcs)
  : Widget::BasicButton(position, main_window, nullptr, draw_funcs),
    func_(new Functor(nullptr))
  { action_func_ = func_; }

  Button::~Button() {
    delete func_;
  }

  void Button::SetClickCallback(IClickCallback* callback) {
    func_->SetCallback(callback);
  }

  Slider::Functor::Functor(ISliderCallback* callback,
                           float range_min, float range_max)
  : old_val_(range_min), range_min_(range_min), range_max_(range_max) {}

  void Slider::Functor::Action() {
    assert(callback_ != nullptr && "Error in plugin, callback is not set");
    assert(scroll_pos_ <= 1.0f);
    float new_val = range_min_ + scroll_pos_ * (range_max_ - range_min_);
    callback_->RespondOnSlide(old_val_, new_val);
    old_val_ = new_val;
  }

  void Slider::Functor::SetCallback(ISliderCallback* callback) {
    assert(callback_ != nullptr && "Error in plugin, callback is not set");
    callback_ = callback;
  }

  Slider::Slider(const Rectangle& position,
                 Widget::MainWindow* main_window,
                 int bound0,
                 int bound1,
                 const Widget::ButtonDrawFunctors& draw_funcs,
                 float range_min,
                 float range_max)
  : Widget::Scroll(position, main_window, ScrollType::kHorizontal, bound0, bound1, nullptr, draw_funcs),
    range_min_(range_min),
    range_max_(range_max),
    func_(new Functor(nullptr, range_min_, range_max_))
  {
    scroll_func_ = func_;
    scroll_func_->SetScrollType(ScrollType::kHorizontal);
  }

  Slider::~Slider() {
    delete func_;
  }

  void Slider::SetSliderCallback(ISliderCallback* callback) {
    dynamic_cast<Functor*>(scroll_func_)->SetCallback(callback);
  }

  float Slider::GetValue() {
    int w = position_.width;
    float pos = (w - bound0_) / (bound1_ - bound0_ - w);
    return range_min_ + pos * (range_max_ - range_min_);
  }

  void Slider::SetValue(float value) {
    float pos = (value - range_min_) / (range_max_ - range_min_);
    assert(pos >= 0);
    assert(pos <= 1);
    int w = position_.width;
    position_.corner.x = bound0_ + (int)(pos * (float)(bound1_ - bound0_ - w));
  }

  uint Slider::GetWidth() {
    return position_.width;
  }

  uint Slider::GetHeight() {
    return position_.height;
  }

  Label::Label(const Point2D<int>& position,
               const char* text,
               Render* render,
               const ::Color& color)
  : UserWidget::Label(position, text, render, color) {}

  uint Label::GetWidth() {
    return position_.width;
  }

  uint Label::GetHeight() {
    return position_.height;
  }

  void Label::SetText(const char* text) {
    UserWidget::Label::SetText(text);
  }

  Icon::Icon(Rectangle position)
  : Widget::Icon(position, nullptr) {}
  Icon::~Icon() { $;delete draw_func_;$$; }

  void Icon::SetIcon(const ITexture* icon) {
    $;
    delete draw_func_;
    ITexture* _icon = const_cast<ITexture*>(icon);
    draw_func_ = new DrawFunctor::ScalableTexture(&(dynamic_cast<Texture*>(_icon)->texture_));
    $$;
  }

  uint Icon::GetWidth() {
    return position_.width;
  }

  uint Icon::GetHeight() {
    return position_.height;
  }

  PreferencesPanel::PreferencesPanel(Widget::MainWindow* main_window,
                                     WidgetFactory* widget_factory)
  : Widget::Container({{0, 0}, kPrefPanelWidth, kPrefPanelHeight}, {}, kFuncDrawTexMain),
    widget_factory_(widget_factory) {}

  void PreferencesPanel::Attach(IButton*  button,  int x, int y) {
    Attach(static_cast<IWidget*>(button), x, y);
  }

  void PreferencesPanel::Attach(ILabel*   label,   int x, int y) {
    Attach(static_cast<IWidget*>(label), x, y);
  }

  void PreferencesPanel::Attach(ISlider*  slider,  int x, int y) {
    Attach(static_cast<IWidget*>(slider), x, y);
  }

  void PreferencesPanel::Attach(IIcon*    icon,    int x, int y) {
    Attach(static_cast<IWidget*>(icon), x, y);
  }

  void PreferencesPanel::Attach(IPalette* palette, int x, int y) {
    widget_factory_->AddPalette(dynamic_cast<Palette*>(palette));
  }

  void PreferencesPanel::Attach(IWidget* iwidget, int x, int y) {
    Widget::Abstract* widget = dynamic_cast<Widget::Abstract*>(iwidget);
    assert(widget != nullptr);
    Rectangle pos = widget->GetPosition();
    pos.corner.x = x + position_.corner.x + kPrefPanelOfs;
    pos.corner.y = y + position_.corner.y;
    widget->SetPosition(pos);
    AddChild(widget);

    Widget::Scroll* w = dynamic_cast<Widget::Scroll*>(iwidget);
    if (w != nullptr) {
      w->SetBound0(pos.corner.x);
      w->SetBound1(position_.corner.x + position_.width - kPrefPanelOfs);
    }
  }

  uint PreferencesPanel::GetWidth() {
    return position_.width;
  }

  uint PreferencesPanel::GetHeight() {
    return position_.height;
  }

  WidgetFactory::WidgetFactory(Widget::MainWindow* main_window, Render* render)
  : main_window_(main_window), render_(render) {}

  IButton* WidgetFactory::CreateDefaultButtonWithIcon(const char* icon_file_name) {
    return CreateButtonWithIcon(kDefaultButtonWidth, kDefaultButtonWidth, icon_file_name);
  }

  IButton* WidgetFactory::CreateButtonWithIcon(uint width, uint height, const char* icon_file_name) {
    auto texture = new ::Texture(icon_file_name, render_);
    auto func_draw_texture = new DrawFunctor::TilingTexture(texture);
    auto func_draw_hover = new DrawFunctor::MultipleFunctors({kFuncDrawTexMainLightExtra, func_draw_texture});
    textures_to_free_.push_back(texture);
    draw_funcs_to_free_.push_back(func_draw_texture);
    draw_funcs_to_free_.push_back(func_draw_hover);
    return new Button({{0, 0}, width, height}, main_window_, {func_draw_texture, func_draw_hover});
  }

  WidgetFactory::~WidgetFactory() {
    for (auto f : textures_to_free_) delete f;
    for (auto f : draw_funcs_to_free_) delete f;
  }

  ISlider* WidgetFactory::CreateDefaultSlider(float range_min, float range_max) {
    return new Slider({{0, 0}, 84, kStandardThumbWidth},
                      main_window_, 0, 0,
                      {kFuncDrawHorizontalScrollBarNormal, kFuncDrawHorizontalScrollBarHover, kFuncDrawHorizontalScrollBarClick},
                      range_min, range_max);
  }

  ILabel* WidgetFactory::CreateDefaultLabel(const char* text) {
    return new Label({0, 0}, text, render_, kWhite);
  }

  IIcon* WidgetFactory::CreateIcon(uint width, uint height) {
    return new Icon({{0, 0}, width, height});
  }

  IPalette* WidgetFactory::CreatePalette() {
    return new Palette;
  }

  IPreferencesPanel* WidgetFactory::CreatePreferencesPanel() {
    return new PreferencesPanel(main_window_, this);
  }

  API::API(Widget::MainWindow* main_window, Render* render)
  : w_factory_(main_window, render), t_factory_(render) {}

  IWidgetFactory* API::GetWidgetFactory() {
    return &w_factory_;
  }

  ITextureFactory* API::GetTextureFactory() {
    return &t_factory_;
  }

  IButton* WidgetFactory::CreateDefaultButtonWithText(const char* text) {
    assert(!"This function is yet to be implemented");
  }
  IButton* WidgetFactory::CreateButtonWithText(uint width, uint height, const char* text, uint char_size) {
    assert(!"This function is yet to be implemented");
  }
  ISlider* WidgetFactory::CreateSlider(uint width, uint height, float range_min, float range_max) {
    assert(!"This function is yet to be implemented");
    // CreateDefaultSlider(range_min, range_max);
  }
  ILabel* WidgetFactory::CreateLabel(uint width, uint height, const char* text, uint char_size) {
    assert(!"This function is yet to be implemented");
    // CreateDefaultLabel(text);
  }
}