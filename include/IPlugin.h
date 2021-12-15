#ifndef PLUGIN_H_INCLUDED
#define PLUGIN_H_INCLUDED

#include <cstdint>
#include <list>

namespace Plugin {

const uint32_t kVersion = 1;
typedef uint32_t Color; // RGBA8888
    
struct ITexture;

struct Buffer {
  Color* pixels;
  ITexture* texture;
};

struct ITexture {
  virtual ~ITexture() = default;

  virtual int32_t GetWidth() = 0;
  virtual int32_t GetHeight() = 0;

  virtual Buffer ReadBuffer() = 0;
  virtual void ReleaseBuffer(Buffer buffer) = 0;
  virtual void LoadBuffer(Buffer buffer) = 0;

  virtual void Clear(Color color) = 0;
  virtual void Present() = 0;

  virtual void DrawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Color color) = 0;
  virtual void DrawThickLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t thickness, Color color) = 0;
  virtual void DrawCircle(int32_t x, int32_t y, int32_t radius, Color color) = 0;
  virtual void DrawRect(int32_t x, int32_t y, int32_t width, int32_t height, Color color) = 0;

  virtual void CopyTexture(ITexture* texture, int32_t x, int32_t y, int32_t width, int32_t height) = 0;
};

struct ITextureFactory {
  virtual ~ITextureFactory() = default;
  virtual ITexture* CreateTexture(const char* filename) = 0;
  virtual ITexture* CreateTexture(int32_t width, int32_t height) = 0;
};

struct IClickCallback {
  virtual ~IClickCallback() = default;
  virtual void RespondOnClick() = 0;
};

struct ISliderCallback {
  virtual ~ISliderCallback() = default;
  virtual void RespondOnSlide(float old_value, float current_value) = 0;
};

struct IWidget {
  virtual ~IWidget() = default;
  virtual int32_t GetWidth() = 0;
  virtual int32_t GetHeight() = 0;
};

struct IButton : public IWidget {
  virtual ~IButton() = default;
  virtual void SetClickCallback(IClickCallback* callback) = 0;
};

struct ISlider : public IWidget {
  virtual ~ISlider() = default;
  virtual void SetSliderCallback(ISliderCallback* callback) = 0;
  virtual float GetValue() = 0;
  virtual void SetValue(float value) = 0;
};

struct ILabel : public IWidget {
  virtual ~ILabel() = default;
  virtual void SetText(const char* text) = 0;
};

struct IPreferencesPanel : public IWidget {
  virtual ~IPreferencesPanel() = default;
  virtual void Attach(IWidget* widget, int32_t x, int32_t y) = 0;
};

struct IWidgetFactory {
  virtual ~IWidgetFactory() = default;

  virtual IButton* CreateDefaultButtonWithIcon(const char* icon_file_name) = 0;
  virtual IButton* CreateDefaultButtonWithText(const char* text) = 0;
  virtual IButton* CreateButtonWithIcon(int32_t width, int32_t height, const char* icon_file_name) = 0;
  virtual IButton* CreateButtonWithText(int32_t width, int32_t height, const char* text, int32_t char_size) = 0;

  virtual ISlider* CreateDefaultSlider(float range_min, float range_max) = 0;
  virtual ISlider* CreateSlider(int32_t width, int32_t height, float range_min, float range_max) = 0;
  virtual ISlider* CreateSlider(int32_t width, int32_t height, float thumb_width, float thumb_height, float range_min, float range_max) = 0;

  virtual ILabel*  CreateDefaultLabel(const char* text) = 0;
  virtual ILabel*  CreateLabel(int32_t width, int32_t height, const char* text, int32_t char_size) = 0;

  virtual IPreferencesPanel* CreateDefaultPreferencesPanel() = 0;
};

struct IAPI {
  virtual ~IAPI() = default;
  virtual IWidgetFactory*  GetWidgetFactory () = 0;
  virtual ITextureFactory* GetTextureFactory() = 0;
};

struct IFilter {
  virtual ~IFilter() = default;
  virtual void Apply(ITexture* canvas) = 0;
  virtual const char* GetName() const = 0;
  virtual IPreferencesPanel* GetPreferencesPanel() const = 0;
};

struct ITool {
  virtual ~ITool() = default;

  virtual void ActionBegin(ITexture* canvas, int x, int y) = 0;
  virtual void Action     (ITexture* canvas, int x, int y, int dx, int dy) = 0;
  virtual void ActionEnd  (ITexture* canvas, int x, int y) = 0;

  virtual const char* GetIconFileName() const = 0;
  virtual IPreferencesPanel* GetPreferencesPanel() const = 0;
};

struct IPlugin {
  virtual ~IPlugin() = default;
  virtual std::list<IFilter*> GetFilters() const = 0;
  virtual std::list<ITool*>   GetTools () const = 0;
};
    
typedef IPlugin*(*CreateFunction)(IAPI* api);
typedef void    (*DestroyFunction)(IPlugin* plugin);
typedef uint32_t(*VersionFunction)();

} // namespace plugin

#endif /* PLUGIN_H_INCLUDED */