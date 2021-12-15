#include <stdio.h>
#include <vector>
#include "../include/IPlugin.h"
#include "../include/main.h"

namespace Plugin {

enum SettingType {
  kSquareSize,
  kRed,
  kGreen,
  kBlue,
  kSettingTypeSize
};

class DrawRectsTool;
class FillingTool;

class Functor : public ISliderCallback {
 public:
  Functor() = delete;
  Functor(SettingType type, DrawRectsTool* tool);

  void RespondOnSlide(float old_value, float current_value) override;

 private:
  SettingType type_;
  DrawRectsTool* tool_;
};

class FunctorForFiller : public ISliderCallback {
 public:
  FunctorForFiller() = delete;
  FunctorForFiller(SettingType type, FillingTool* tool);

  void RespondOnSlide(float old_value, float current_value) override;

 private:
  SettingType type_;
  FillingTool* tool_;
};


class DrawRectsTool : public ITool {
 public:
  DrawRectsTool() = delete;
  DrawRectsTool(IAPI* api);
  ~DrawRectsTool() override;

  void ActionBegin(ITexture* canvas, int x, int y) override;
  void Action(ITexture* canvas, int x, int y, int dx, int dy) override;
  void ActionEnd  (ITexture* canvas, int x, int y) override {}
  const char* GetIconFileName() const override;
  void SettingHasChanged(SettingType type);
  uint CreateSlider(IPreferencesPanel* pref_panel, int x, int y,
                    const char* text, float min, float max,
                    SettingType type);
  IPreferencesPanel* GetPreferencesPanel() const override;

 private:
  IWidgetFactory* widget_factory_;
  std::vector<Functor*> funcs_to_free_;

  ILabel* numbers_labels_[kSettingTypeSize];
};

class FillingTool : public ITool {
 public:
  FillingTool() = delete;
  FillingTool(IAPI* api);
  ~FillingTool() override;

  void ActionBegin(ITexture* canvas, int x, int y) override;
  void Action(ITexture* canvas, int x, int y, int dx, int dy) override {}
  void ActionEnd  (ITexture* canvas, int x, int y) override {}
  const char* GetIconFileName() const override;
  void SettingHasChanged(SettingType type);
  uint CreateSlider(IPreferencesPanel* pref_panel, int x, int y,
                    const char* text, float min, float max,
                    SettingType type);
  IPreferencesPanel* GetPreferencesPanel() const override;

 private:
  IWidgetFactory* widget_factory_;
  std::vector<FunctorForFiller*> funcs_to_free_;

  ILabel* numbers_labels_[kSettingTypeSize];
};

class FilterInverse : public IFilter {
 public:
  FilterInverse() = delete;
  FilterInverse(IAPI* api);
  ~FilterInverse() override = default;
  void Apply(ITexture* canvas) override;
  const char* GetName() const override;
  IPreferencesPanel* GetPreferencesPanel() const override;

 private:
  IWidgetFactory* widget_factory_;
};

class Gradient : public IFilter {
 public:
  Gradient() = delete;
  Gradient(IAPI* api);
  ~Gradient() override = default;
  void Apply(ITexture* canvas) override;
  const char* GetName() const override;
  IPreferencesPanel* GetPreferencesPanel() const override;

 private:
  IWidgetFactory* widget_factory_;
};

}