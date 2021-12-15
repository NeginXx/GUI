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

  int Texture::GetWidth() {
  	return texture_.GetWidth();
  }

  int Texture::GetHeight() {
  	return texture_.GetHeight();
  }

  Buffer Texture::ReadBuffer() {
    Color* buffer = new Color[GetWidth() * GetHeight()];
    SDL_SetRenderTarget(render_->render_, texture_.texture_);
    assert(!SDL_RenderReadPixels(render_->render_, NULL, SDL_PIXELFORMAT_RGBA8888,
                                 buffer, GetWidth() * sizeof(Color)));
    return {buffer, this};
  }

  void Texture::LoadBuffer(Buffer buffer) {
    assert(!SDL_UpdateTexture(texture_.texture_, NULL, buffer.pixels, GetWidth() * sizeof(Color)));
  }

  void Texture::ReleaseBuffer(Buffer buffer) {
    delete[] buffer.pixels;
  }

  void Texture::Clear(Plugin::Color color) {
    texture_.SetBackgroundColor(GetColor(color));
  }

  void Texture::DrawLine(int x0, int y0, int x1, int y1, Plugin::Color color) {
    texture_.DrawLine(Point2D<int>{x0, y0}, Point2D<int>{x1, y1}, GetColor(color));
  }

  void Texture::DrawThickLine(int x0, int y0, int x1, int y1, int thickness, Plugin::Color color) {
    assert(thickness >= 0);
    texture_.DrawThickLine(Point2D<int>{x0, y0}, Point2D<int>{x1, y1}, (uint)thickness, GetColor(color));
  }

  void Texture::DrawCircle(int x, int y, int radius, Plugin::Color color) {
    assert(radius >= 0);
    texture_.DrawCircle(Point2D<int>{x, y}, (uint)radius, GetColor(color));
  }

  void Texture::DrawRect(int x, int y, int width, int height, Plugin::Color color) {
    assert(width >= 0);
    assert(height >= 0);
    texture_.DrawRect({{x, y}, (uint)width, (uint)height}, GetColor(color));
  }

  void Texture::CopyTexture(ITexture* texture, int x, int y, int width, int height) {
    assert(width >= 0);
    assert(height >= 0);
    Rectangle dst = {{x, y}, (uint)width, (uint)height};
    texture_.CopyTexture(dynamic_cast<Texture*>(texture)->texture_, &dst);
  }

  void Texture::Draw(const Rectangle& position, const Point2D<int>& src) {
    texture_.DrawWithNoScale(&position, src);
  }

  TextureFactory::TextureFactory(Render* render)
  : render_(render) {}

  ITexture* TextureFactory::CreateTexture(const char* filename) {
    return new Texture(filename, render_);
  }

  ITexture* TextureFactory::CreateTexture(int32_t width, int32_t height) {
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

  int32_t Button::GetWidth() {
    return position_.width;
  }

  int32_t Button::GetHeight() {
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

  int32_t Slider::GetWidth() {
    return position_.width;
  }

  int32_t Slider::GetHeight() {
    return position_.height;
  }

  Label::Label(const Point2D<int>& position,
               const char* text,
               Render* render,
               const ::Color& color)
  : UserWidget::Label(position, text, render, color) {}

  int32_t Label::GetWidth() {
    return position_.width;
  }

  int32_t Label::GetHeight() {
    return position_.height;
  }

  void Label::SetText(const char* text) {
    UserWidget::Label::SetText(text);
  }

  PreferencesPanel::PreferencesPanel(Widget::MainWindow* main_window)
  : Widget::Container({{0, 0}, kPrefPanelWidth, kPrefPanelHeight}, {}, kFuncDrawTexMain) {}

  void PreferencesPanel::Attach(IWidget* iwidget, int32_t x, int32_t y) {
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

  int32_t PreferencesPanel::GetWidth() {
    return position_.width;
  }

  int32_t PreferencesPanel::GetHeight() {
    return position_.height;
  }

  WidgetFactory::WidgetFactory(Widget::MainWindow* main_window, Render* render)
  : main_window_(main_window), render_(render) {}

  IButton* WidgetFactory::CreateDefaultButtonWithIcon(const char* icon_file_name) {
    return CreateButtonWithIcon(kDefaultButtonWidth, kDefaultButtonWidth, icon_file_name);
  }

  IButton* WidgetFactory::CreateButtonWithIcon(int32_t width, int32_t height, const char* icon_file_name) {
    auto texture = new ::Texture(icon_file_name, render_);
    auto func_draw_texture = new DrawFunctor::TilingTexture(texture);
    auto func_draw_hover = new DrawFunctor::MultipleFunctors({kFuncDrawTexMainLightExtra, func_draw_texture});
    textures_to_free_.push_back(texture);
    draw_funcs_to_free_.push_back(func_draw_texture);
    draw_funcs_to_free_.push_back(func_draw_hover);
    return new Button({{0, 0}, (uint)width, (uint)height}, main_window_, {func_draw_texture, func_draw_hover});
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

  IPreferencesPanel* WidgetFactory::CreateDefaultPreferencesPanel() {
    return new PreferencesPanel(main_window_);
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
  IButton* WidgetFactory::CreateButtonWithText(int32_t width, int32_t height, const char* text, int32_t char_size) {
    assert(!"This function is yet to be implemented");
  }
  ISlider* WidgetFactory::CreateSlider(int32_t width, int32_t height, float range_min, float range_max) {
    assert(!"This function is yet to be implemented");
  }

  ISlider* WidgetFactory::CreateSlider(int32_t width, int32_t height, float thumb_width, float thumb_height, float range_min, float range_max) {
    assert(!"This function is yet to be implemented");
  }
  ILabel* WidgetFactory::CreateLabel(int32_t width, int32_t height, const char* text, int32_t char_size) {
    assert(!"This function is yet to be implemented");
  }
}