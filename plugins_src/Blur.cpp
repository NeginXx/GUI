typedef unsigned int uint;
#include "../include/IPlugin.h"
#include <cstdlib>

namespace Plugin {

class BlurFilter : public IFilter {
public:
    BlurFilter(Plugin::IAPI* api)
        : panel_{nullptr} {
        panel_ = api->GetWidgetFactory()->CreatePreferencesPanel();
    }

    ~BlurFilter() {
        delete panel_;
    }

    virtual void Apply(ITexture* canvas) override {

        int32_t width  = canvas->GetWidth();
        int32_t height = canvas->GetHeight();

        Buffer old_buffer = canvas->ReadBuffer();
        Buffer buffer     = canvas->ReadBuffer();

        for (int32_t x = 1; x < width - 1; ++x) {
            for (int32_t y = 1; y < height - 1; ++y) {
                uint32_t r_sum = GetR(old_buffer.pixels[x - 1 + (y + 1) * width])+
                                 GetR(old_buffer.pixels[x + 0 + (y + 1) * width])+
                                 GetR(old_buffer.pixels[x + 1 + (y + 1) * width])+
                                 GetR(old_buffer.pixels[x - 1 + (y + 0) * width])+
                                 GetR(old_buffer.pixels[x + 0 + (y + 0) * width])+
                                 GetR(old_buffer.pixels[x + 1 + (y + 0) * width])+
                                 GetR(old_buffer.pixels[x - 1 + (y - 1) * width])+
                                 GetR(old_buffer.pixels[x + 0 + (y - 1) * width])+
                                 GetR(old_buffer.pixels[x + 1 + (y - 1) * width]);

                uint32_t g_sum = GetG(old_buffer.pixels[x - 1 + (y + 1) * width]) +
                                 GetG(old_buffer.pixels[x + 0 + (y + 1) * width]) +
                                 GetG(old_buffer.pixels[x + 1 + (y + 1) * width]) +
                                 GetG(old_buffer.pixels[x - 1 + (y + 0) * width]) +
                                 GetG(old_buffer.pixels[x + 0 + (y + 0) * width]) +
                                 GetG(old_buffer.pixels[x + 1 + (y + 0) * width]) +
                                 GetG(old_buffer.pixels[x - 1 + (y - 1) * width]) +
                                 GetG(old_buffer.pixels[x + 0 + (y - 1) * width]) +
                                 GetG(old_buffer.pixels[x + 1 + (y - 1) * width]);

                uint32_t b_sum = GetB(old_buffer.pixels[x - 1 + (y + 1) * width]) +
                                 GetB(old_buffer.pixels[x + 0 + (y + 1) * width]) +
                                 GetB(old_buffer.pixels[x + 1 + (y + 1) * width]) +
                                 GetB(old_buffer.pixels[x - 1 + (y + 0) * width]) +
                                 GetB(old_buffer.pixels[x + 0 + (y + 0) * width]) +
                                 GetB(old_buffer.pixels[x + 1 + (y + 0) * width]) +
                                 GetB(old_buffer.pixels[x - 1 + (y - 1) * width]) +
                                 GetB(old_buffer.pixels[x + 0 + (y - 1) * width]) +
                                 GetB(old_buffer.pixels[x + 1 + (y - 1) * width]);
                
                uint32_t a_sum = GetA(old_buffer.pixels[x - 1 + (y + 1) * width]) +
                                 GetA(old_buffer.pixels[x + 0 + (y + 1) * width]) +
                                 GetA(old_buffer.pixels[x + 1 + (y + 1) * width]) +
                                 GetA(old_buffer.pixels[x - 1 + (y + 0) * width]) +
                                 GetA(old_buffer.pixels[x + 0 + (y + 0) * width]) +
                                 GetA(old_buffer.pixels[x + 1 + (y + 0) * width]) +
                                 GetA(old_buffer.pixels[x - 1 + (y - 1) * width]) +
                                 GetA(old_buffer.pixels[x + 0 + (y - 1) * width]) +
                                 GetA(old_buffer.pixels[x + 1 + (y - 1) * width]);

                buffer.pixels[x + y * width] = GetRGBA(r_sum / 9, g_sum / 9, b_sum / 9, a_sum / 9);
            }
        }        

        canvas->LoadBuffer(buffer);

        canvas->ReleaseBuffer(buffer);
        canvas->ReleaseBuffer(old_buffer);
    }

    virtual const char* GetName() const override {
        return "Blur";
    }

    virtual IPreferencesPanel* GetPreferencesPanel() const override {
        return panel_;
    }

public:
    uint32_t GetR(uint32_t color) {
        return (color >> 24) & 255;
    }

    uint32_t GetG(uint32_t color) {
        return (color >> 16) & 255;
    }

    uint32_t GetB(uint32_t color) {
        return (color >>  8) & 255;
    }

    uint32_t GetA(uint32_t color) {
        return (color      ) & 255;
    }

    uint32_t GetRGBA(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
        return (r << 24) + (g << 16) + (b <<  8) + a;
    }

private:
    IPreferencesPanel* panel_;
};

class Plugin : public IPlugin {
public:
    Plugin(IAPI* api)
        : blur_filter_{nullptr} {
        blur_filter_ = new BlurFilter(api);
    }

    virtual ~Plugin() override {}

    virtual Filters GetFilters() const override {
        Filters filter_list;
        filter_list.count = 1;
        filter_list.filters = new IFilter*[filter_list.count];
        filter_list.filters[0] = blur_filter_;
        return filter_list;
    }

    virtual Tools GetTools() const override {
        Tools tool_list;
        tool_list.count = 0;
        return tool_list;
    }

private:
    BlurFilter* blur_filter_;
};

extern "C" IPlugin* Create(IAPI* api) {
    return new Plugin(api);
}

extern "C" void Destroy(IPlugin* plugin) {
    delete plugin;
}

extern "C" uint32_t Version() {
    return kVersion;
}

} // namespace plugin