#include "Plugin.h"
#include <stdio.h>

namespace Plugin {
  struct DrawRectsTool : ITool {
  	DrawRectsTool(): width(10) {}
    ~DrawRectsTool() override = default;

    void ActionBegin(ITexture* canvas, int x, int y) override {
      int xx = x - width / 2;
      int yy = y - width / 2;
      canvas->DrawRect(xx, yy, width, width, 0x000000FF);
      canvas->DrawRect(xx + 1, yy + 1, width - 2, width - 2, 0xFFFFFFFF);
    }

    void Action(ITexture* canvas, int x, int y, int dx, int dy) override {
      ActionBegin(canvas, x + dx, y + dy);
    }

    void ActionEnd  (ITexture* canvas, int x, int y) override {}

    virtual const char* GetIconFileName() const override {
    	return "tool_square.png";
    }

    virtual IPreferencesPanel* GetPreferencesPanel() const override {
    	return nullptr;
    }

    unsigned int width;
  };

  struct MyPlugin : public IPlugin {
   	MyPlugin(IAPI* api)
    : tool_(new DrawRectsTool()) {}

    ~MyPlugin() override {}

   	std::list<IFilter*> GetFilters() const override {
   		return {};
   	}

   	std::list<ITool*> GetTools() const override {
   		return {tool_};
   	}

   private:
    DrawRectsTool* tool_;
  };

  MyPlugin* kPlugin = nullptr;

  extern "C" IPlugin* Create(IAPI* api) {
  	kPlugin = new MyPlugin(api);
    return kPlugin;
  }

  extern "C" void Destroy(IPlugin* plugin) {
    delete kPlugin;
  }

  extern "C" uint32_t Version() {
    return 1;
  }
}