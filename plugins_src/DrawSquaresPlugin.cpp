#include "DrawSquaresPlugin.h"

uint kPrefPanelWidth = 0;
const uint kPrefPanelOfs = 10;
const uint kSpacing = 7;

namespace Plugin {

unsigned char kSettings[kSettingTypeSize] = {2, 0, 0, 0};
unsigned char kSettingsForFilling[kSettingTypeSize] = {2, 0, 0, 0};

class ChangeColorCallback : public IPaletteCallback {
 public:
  ChangeColorCallback() = default;
  void RespondOnChangeColor(uint color) override {
    unsigned char alpha = 0xFF;
    unsigned char blue = (color & 0xFF00) >> 8;
    unsigned char green = (color &  0xFF0000) >> 16;
    unsigned char red =   (color & 0xFF000000) >> 24;
    printf("red = %u\n", red);
    printf("green = %u\n", green);
    printf("blue = %u\n", blue);
    kSettings[kRed] = red;
    kSettings[kBlue] = blue;
    kSettings[kGreen] = green;
    kSettingsForFilling[kRed] = red;
    kSettingsForFilling[kBlue] = blue;
    kSettingsForFilling[kGreen] = green;
  }
};

ChangeColorCallback kColorCallback;

Functor::Functor(SettingType type, DrawRectsTool* tool)
: type_(type), tool_(tool) {}

void Functor::RespondOnSlide(float old_value, float current_value) {
  assert(current_value >= 0);
  assert(type_ < kSettingTypeSize);
  kSettings[type_] = (uint)current_value;
  tool_->SettingHasChanged(type_);
}

FunctorForFiller::FunctorForFiller(SettingType type, FillingTool* tool)
: type_(type), tool_(tool) {}

void FunctorForFiller::RespondOnSlide(float old_value, float current_value) {
  assert(current_value >= 0);
  assert(type_ < kSettingTypeSize);
  kSettingsForFilling[type_] = (uint)current_value;
  tool_->SettingHasChanged(type_);
}

DrawRectsTool::DrawRectsTool(IAPI* api)
: widget_factory_(api->GetWidgetFactory()) {}

DrawRectsTool::~DrawRectsTool() {
  for (auto f : funcs_to_free_) delete f;
}

void DrawRectsTool::ActionBegin(ITexture* canvas, int x, int y) {
  uint width = kSettings[kSquareSize];
  int xx = x - width / 2;
  int yy = y - width / 2;
  canvas->DrawRect({xx, yy, width, width, 0, 0xFF000000, 0});
  uint blue = kSettings[kBlue];
  uint green = kSettings[kGreen];
  uint red = kSettings[kRed];
  canvas->DrawRect({xx + 1, yy + 1, width - 2, width - 2, 0, red + (green << 8) + (blue << 16) + (0xFF << 24), 0});
}

void DrawRectsTool::Action(ITexture* canvas, int x, int y, int dx, int dy) {
  ActionBegin(canvas, x + dx, y + dy);
}

const char* DrawRectsTool::GetIconFileName() const {
	return "tool_square.png";
}

void DrawRectsTool::SettingHasChanged(SettingType type) {
  char buf[10] = {};
  sprintf(buf, "%u", kSettings[type]);
  numbers_labels_[type]->SetText(buf);
}

uint DrawRectsTool::CreateSlider(IPreferencesPanel* pref_panel, int x, int y,
                                 const char* text, float min, float max,
                                 SettingType type) {
  uint height = 0;
  ILabel* label = widget_factory_->CreateDefaultLabel(text);
  pref_panel->Attach(label, x, y);
  height += label->GetHeight();

  char buf[10] = {};
  sprintf(buf, "%u", kSettings[type]);
  numbers_labels_[type] = widget_factory_->CreateDefaultLabel(buf);
  pref_panel->Attach(numbers_labels_[type], kPrefPanelWidth - 50, y);
  
  ISlider* slider = widget_factory_->CreateDefaultSlider(min, max);
  auto func = new Functor(type, this);
  slider->SetSliderCallback(func);
  pref_panel->Attach(slider, 0, y + height);
  height += slider->GetHeight();
  funcs_to_free_.push_back(func);

  height += kSpacing;
  IButton* button = widget_factory_->CreateButtonWithIcon(kPrefPanelWidth - 2 * kPrefPanelOfs, 1, "tex_black.png");
  pref_panel->Attach(button, 0, y + height);
  height += button->GetHeight();


  return height;
}

IPreferencesPanel* DrawRectsTool::GetPreferencesPanel() const {
  IPreferencesPanel* pref_panel = widget_factory_->CreatePreferencesPanel();
  kPrefPanelWidth = pref_panel->GetWidth();
  int x = 0;
  int y = 0;
  int height = 0;
  #define CREATE_SLIDER(args...) \
    height += const_cast<DrawRectsTool*>(this)->CreateSlider(pref_panel, x, args);
  
    CREATE_SLIDER(y, "Square size:", 2.0f, 30.0f, kSquareSize);
    CREATE_SLIDER(y + height, "Red:", 0.0f, 255.0f, kRed);
    CREATE_SLIDER(y + height, "Green:", 0.0f, 255.0f, kGreen);
    CREATE_SLIDER(y + height, "Blue:", 0.0f, 255.0f, kBlue);
  #undef CREATE_SLIDER
  IPalette* palette = widget_factory_->CreatePalette();
  palette->SetPaletteCallback(&kColorCallback);
  pref_panel->Attach(palette, 0, 0);
	return pref_panel;
}

FillingTool::FillingTool(IAPI* api)
: widget_factory_(api->GetWidgetFactory()),
  texture_factory_(api->GetTextureFactory()) {}

FillingTool::~FillingTool() {
  for (auto f : funcs_to_free_) delete f;
}

void FillingTool::ActionBegin(ITexture* canvas, int xx, int yy) {
  Buffer buffer = canvas->ReadBuffer();
  Color* buf = buffer.pixels;
  int width = canvas->GetWidth();
  int height = canvas->GetHeight();
  Color color = buf[yy * width + xx];

  uint blue = kSettingsForFilling[kBlue];
  uint green = kSettingsForFilling[kGreen];
  uint red = kSettingsForFilling[kRed];
  uint alpha = 0xFF;
  // Color color_to_draw = red + (green << 8) + (blue << 16) + (alpha << 24);
  Color color_to_draw = alpha + (blue << 8) + (green << 16) + (red << 24);

  int cnt = 0;
  for (int i = 0; i < width * height; ++i) {
    if (buf[i] == color) {
      buf[i] = color_to_draw;
      ++cnt;
    }
  }
  canvas->LoadBuffer(buffer);
  canvas->ReleaseBuffer(buffer);

  ITexture* tex = texture_factory_->CreateTexture("photo.jpg"); ///////////////////////////////////
  canvas->CopyTexture(tex, 0, 0);
}

const char* FillingTool::GetIconFileName() const {
  return "tool_filler.png";
}

void FillingTool::SettingHasChanged(SettingType type) {
  char buf[10] = {};
  sprintf(buf, "%u", kSettingsForFilling[type]);
  numbers_labels_[type]->SetText(buf);
}

uint FillingTool::CreateSlider(IPreferencesPanel* pref_panel, int x, int y,
                              const char* text, float min, float max,
                              SettingType type) {
  uint height = 0;
  ILabel* label = widget_factory_->CreateDefaultLabel(text);
  pref_panel->Attach(label, x, y);
  height += label->GetHeight();

  char buf[10] = {};
  sprintf(buf, "%u", kSettings[type]);
  numbers_labels_[type] = widget_factory_->CreateDefaultLabel(buf);
  pref_panel->Attach(numbers_labels_[type], kPrefPanelWidth - 50, y);
  
  ISlider* slider = widget_factory_->CreateDefaultSlider(min, max);
  auto func = new FunctorForFiller(type, this);
  slider->SetSliderCallback(func);
  pref_panel->Attach(slider, 0, y + height);
  height += slider->GetHeight();
  funcs_to_free_.push_back(func);

  height += kSpacing;
  IButton* button = widget_factory_->CreateButtonWithIcon(kPrefPanelWidth - 2 * kPrefPanelOfs, 1, "tex_black.png");
  pref_panel->Attach(button, 0, y + height);
  height += button->GetHeight();

  return height;
}

IPreferencesPanel* FillingTool::GetPreferencesPanel() const {
  IPreferencesPanel* pref_panel = widget_factory_->CreatePreferencesPanel();
  kPrefPanelWidth = pref_panel->GetWidth();
  int x = 0;
  int y = 0;
  int height = 0;
  #define CREATE_SLIDER(args...) \
    height += const_cast<FillingTool*>(this)->CreateSlider(pref_panel, x, args);
  
    CREATE_SLIDER(y + height, "Red:", 0.0f, 255.0f, kRed);
    CREATE_SLIDER(y + height, "Green:", 0.0f, 255.0f, kGreen);
    CREATE_SLIDER(y + height, "Blue:", 0.0f, 255.0f, kBlue);
  #undef CREATE_SLIDER
  IPalette* palette = widget_factory_->CreatePalette();
  palette->SetPaletteCallback(&kColorCallback);
  pref_panel->Attach(palette, 0, 0);
  return pref_panel;
}

Color InvertttColor(Color color) {
  // uint alpha = color & 0xFF;
  // uint blue = color & 0xFF00;
  // uint green = color & 0xFF0000;
  // uint red = color & 0xFF000000;
  // red = 255 - red;
  // green = 255 - green;
  // blue = 255 - blue;
  // alpha = 0xFF;
  // return alpha + (blue << 8) + (green << 16) + (red << 24);
  return (0xFF'FF'FF'FF - color) | 0x00'00'00'FF;
}

Color GradColor(Color color) {
  uint red   = (uint)pow(color & 0xFF, 0.9);
  uint green = (uint)pow((color >> 8) & 0xFF, 0.9);
  uint blue  = (uint)pow((color >> 16) & 0xFF, 0.9);
  uint alpha = 0xFF;
  return alpha + (blue << 8) + (green << 16) + (red << 24);
}

FilterInverse::FilterInverse(IAPI* api)
: widget_factory_(api->GetWidgetFactory()) {}

void FilterInverse::Apply(ITexture* canvas) {
  Buffer buffer = canvas->ReadBuffer();
  Color* buf = buffer.pixels;
  int width = canvas->GetWidth();
  int height = canvas->GetHeight();

  for (int i = 0; i < width * height; ++i) {
    buf[i] = InvertttColor(buf[i]);
  }
  canvas->LoadBuffer(buffer);
  canvas->ReleaseBuffer(buffer);
}

const char* FilterInverse::GetName() const {
  return "Inverse filter";
}

IPreferencesPanel* FilterInverse::GetPreferencesPanel() const {
  return nullptr;
}

Gradient::Gradient(IAPI* api)
: widget_factory_(api->GetWidgetFactory()) {}

void Gradient::Apply(ITexture* canvas) {
  Buffer buffer = canvas->ReadBuffer();
  Color* buf = buffer.pixels;
  int width = canvas->GetWidth();
  int height = canvas->GetHeight();

  for (int i = 0; i < width * height; ++i) {
    buf[i] = GradColor(buf[i]);
  }
  canvas->LoadBuffer(buffer);
  canvas->ReleaseBuffer(buffer);
}

const char* Gradient::GetName() const {
  return "Gradient";
}

IPreferencesPanel* Gradient::GetPreferencesPanel() const {
  return nullptr;
}

struct MyPlugin : public IPlugin {
 	MyPlugin(IAPI* api)
  : draw_rects_tool_(new DrawRectsTool(api)),
    filling_tool_(new FillingTool(api)),
    filter_inverse_(new FilterInverse(api)),
    filter_gradient_(new Gradient(api)),
    tools_({draw_rects_tool_, filling_tool_}),
    filters_({filter_inverse_, filter_gradient_}),
    tools__(new ITool*[tools_.size()]),
    filters__(new IFilter*[filters_.size()])
  {
    for (uint i = 0; i < tools_.size(); ++i) {
      tools__[i] = tools_[i];
    }
    for (uint i = 0; i < filters_.size(); ++i) {
      filters__[i] = filters_[i];
    }
  }

  ~MyPlugin() override {
    delete draw_rects_tool_;
    delete[] tools__;
    delete[] filters__;
  }

 	Filters GetFilters() const override {
    return {filters__, (uint)filters_.size()};
 	}

 	Tools GetTools() const override {
 		return {tools__, (uint)tools_.size()};
 	}

 private:
  DrawRectsTool* draw_rects_tool_;
  FillingTool* filling_tool_;
  FilterInverse* filter_inverse_;
  Gradient* filter_gradient_;
  std::vector<ITool*> tools_;
  std::vector<IFilter*> filters_;
  ITool** tools__;
  IFilter** filters__;
};

MyPlugin* kPlugin = nullptr;

extern "C" IPlugin* Create(IAPI* api) {
	kPlugin = new MyPlugin(api);
  return kPlugin;
}

extern "C" void Destroy(IPlugin* plugin) {
  delete kPlugin;
}

extern "C" uint Version() {
  return 2;
}

}