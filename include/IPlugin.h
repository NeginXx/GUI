#ifndef _PLUGIN_HPP_INCLUDED_
#define _PLUGIN_HPP_INCLUDED_

#include <cstdint>

namespace Plugin {

const uint kVersion = 2; // updated version
typedef uint Color;      // Color = 0xAA'BB'GG'RR;

struct ITexture;

struct Buffer {
  Color* pixels;
  ITexture* texture;
};

struct Rect {
  int x;
  int y;
  uint width;
  uint height; 
  uint outline_thickness;
  Color fill_color;
  Color outline_color;
};

struct Circle {
  int x;
  int y;
  uint radius;
  uint outline_thickness;
  Color fill_color;
  Color outline_color;
};

struct Line {
  int x0;
  int y0;
  int x1;
  int y1;
  uint thickness;
  Color color;
};

struct ITexture {
  virtual ~ITexture() {}

  virtual uint GetWidth() = 0;
  virtual uint GetHeight() = 0;

  virtual Buffer ReadBuffer() = 0;
  virtual void ReleaseBuffer(Buffer buffer) = 0;
  virtual void LoadBuffer(Buffer buffer) = 0;

  virtual void Clear(Color Color) = 0;
  virtual void Present() = 0;

  virtual void DrawLine  (const Line& line) = 0;
  virtual void DrawCircle(const Circle& circle) = 0;
  virtual void DrawRect  (const Rect& rect) = 0;

  virtual void CopyTexture(ITexture* source, int x, int y, uint width, uint height) = 0;
  virtual void CopyTexture(ITexture* source, int x, int y) = 0;
};

struct ITextureFactory {
  virtual ~ITextureFactory() {}
  virtual ITexture* CreateTexture(const char* filename) = 0;
  virtual ITexture* CreateTexture(uint width, uint height) = 0;
};

struct IClickCallback {
  virtual ~IClickCallback() {}
  virtual void RespondOnClick() = 0;
};

struct ISliderCallback {
  virtual ~ISliderCallback() {}
  virtual void RespondOnSlide(float old_value, float current_value) = 0;
};

struct IPaletteCallback {
  virtual ~IPaletteCallback() {}
  virtual void RespondOnChangeColor(Color color) = 0;  
};

struct IWidget {
  virtual ~IWidget() {}
  virtual uint GetWidth() = 0;
  virtual uint GetHeight() = 0;
};

struct IButton : public IWidget {
  virtual ~IButton() {}
  virtual void SetClickCallback(IClickCallback* callback) = 0;
};

struct ISlider : public IWidget {
  virtual ~ISlider() {}
  virtual void SetSliderCallback(ISliderCallback* callback) = 0;
  virtual float GetValue() = 0;
  virtual void SetValue(float value) = 0;
};

struct ILabel : public IWidget {
  virtual ~ILabel() {}
  virtual void SetText(const char* text) = 0;
};

struct IIcon : public IWidget {
  virtual ~IIcon() {}
  virtual void SetIcon(const ITexture* icon) = 0;
};

struct IPalette : public IWidget {
  virtual ~IPalette() {}
  virtual void SetPaletteCallback(IPaletteCallback* callback) = 0;
};

struct IPreferencesPanel : public IWidget {
  virtual ~IPreferencesPanel() {}
  virtual void Attach(IButton*  button,  int x, int y) = 0;
  virtual void Attach(ILabel*   label,   int x, int y) = 0;
  virtual void Attach(ISlider*  slider,  int x, int y) = 0;
  virtual void Attach(IIcon*    icon,    int x, int y) = 0;
  virtual void Attach(IPalette* palette, int x, int y) = 0;
};

struct IWidgetFactory {
  virtual ~IWidgetFactory() {}

  virtual IButton* CreateDefaultButtonWithIcon(const char* icon_file_name) = 0;
  virtual IButton* CreateDefaultButtonWithText(const char* text) = 0;
  virtual IButton* CreateButtonWithIcon(uint width, uint height, const char* icon_file_name) = 0;
  virtual IButton* CreateButtonWithText(uint width, uint height, const char* text, uint char_size) = 0;

  virtual ISlider* CreateDefaultSlider(float range_min, float range_max) = 0;
  virtual ISlider* CreateSlider(uint width, uint height, float range_min, float range_max) = 0;

  virtual ILabel*  CreateDefaultLabel(const char* text) = 0;
  virtual ILabel*  CreateLabel(uint width, uint height, const char* text, uint char_size) = 0;

  virtual IIcon*   CreateIcon(uint width, uint height) = 0;

  virtual IPalette* CreatePalette() = 0;

  virtual IPreferencesPanel* CreatePreferencesPanel() = 0;
};

struct IAPI {
  virtual ~IAPI() {}

  virtual IWidgetFactory*  GetWidgetFactory () = 0;
  virtual ITextureFactory* GetTextureFactory() = 0;
};

struct IFilter {
  virtual ~IFilter() {}

  virtual void Apply(ITexture* canvas) = 0;
  virtual const char* GetName() const = 0;

  virtual IPreferencesPanel* GetPreferencesPanel() const = 0;
};

struct ITool {
  virtual ~ITool() {}

  virtual void ActionBegin(ITexture* canvas, int x, int y) = 0;
  virtual void Action     (ITexture* canvas, int x, int y, int dx, int dy) = 0;
  virtual void ActionEnd  (ITexture* canvas, int x, int y) = 0;

  virtual const char* GetIconFileName() const = 0;
  virtual const char* GetName() const = 0;
  virtual IPreferencesPanel* GetPreferencesPanel() const = 0;
};

struct Tools {
  ITool** tools;
  uint count;
};

struct Filters {
  IFilter** filters;
  uint count;
};

struct IPlugin {
  virtual ~IPlugin() {}
  virtual Filters GetFilters() const = 0;
  virtual Tools   GetTools()   const = 0;
};

typedef IPlugin* (*CreateFunction) (IAPI* api);
typedef void     (*DestroyFunction)(IPlugin* plugin);
typedef uint (*VersionFunction)();

#ifdef _WIN32 //windows

#define TOOLCALL __cdecl

#ifdef EXPORT_TOOL
#define TOOLAPI __declspec(dllexport)
#else
#define TOOLAPI __declspec(dllimport)
#endif

extern "C" TOOLAPI IPlugin* TOOLCALL Create(IAPI* api);
extern "C" TOOLAPI void     TOOLCALL Destroy(IPlugin* plugin);
extern "C" TOOLAPI uint TOOLCALL Version();

#endif

} // namespace plugin

#endif /* _PLUGIN_HPP_INCLUDED_ */