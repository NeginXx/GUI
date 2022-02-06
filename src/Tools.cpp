#include <stdio.h>
#include <dlfcn.h>
#include "../include/Tools.h"
#include "../include/GUIConstants.h"
#include "../include/Plugin.h"

extern Plugin::API* kApi;

namespace Tool {
	Manager* Manager::GetInstance() {
		static Manager* instance = nullptr;
		static bool is_called_first_time = true;
		if (is_called_first_time) {
			is_called_first_time = false;
			instance = new Manager();
		 	instance->Init();
		}
	  return instance;
	}

  Manager::Manager()
  : thickness_(3),
    color_(kBlack) {}

	void Manager::Init() {
		char temp[100] = {};
		sprintf(temp, "%s/%s", kPluginsDirName, "DrawSquaresPlugin.so");
		plugin_lib_ = dlopen(temp, RTLD_NOW);
		assert(plugin_lib_ != nullptr);
		Plugin::CreateFunction Create = (Plugin::CreateFunction)dlsym(plugin_lib_, "Create");
		assert(Create != nullptr);
		plugin_ = Create(kApi);

		Plugin::Tools tools_temp = plugin_->GetTools();
		for (uint i = 0; i < tools_temp.count; ++i) {tools_.push_front(tools_temp.tools[i]);}
		tools_.push_front(new Pencil());
		tools_.push_front(new Eraser());
		cur_tool_ = *tools_.begin();

		Plugin::Filters f_temp = plugin_->GetFilters();
		for (uint i = 0; i < f_temp.count; ++i) {filters_.push_front(f_temp.filters[i]);}


		memset(temp, 0, sizeof(temp));
		sprintf(temp, "%s/%s", kPluginsDirName, "Blur.so");
		plugin_lib_ = dlopen(temp, RTLD_NOW);
		assert(plugin_lib_ != nullptr);
		Plugin::CreateFunction Create1 = (Plugin::CreateFunction)dlsym(plugin_lib_, "Create");
		assert(Create1 != nullptr);
		plugin_ = Create1(kApi);

		Plugin::Tools tools_temp1 = plugin_->GetTools();
		for (uint i = 0; i < tools_temp1.count; ++i) {tools_.push_front(tools_temp1.tools[i]);}

		Plugin::Filters f_temp1 = plugin_->GetFilters();
		for (uint i = 0; i < f_temp1.count; ++i) {filters_.push_front(f_temp1.filters[i]);}
	}

	Manager::~Manager() {
		delete *tools_.begin();
		delete *++tools_.begin();
		Plugin::DestroyFunction Destroy = (Plugin::DestroyFunction)dlsym(plugin_lib_, "Destroy");
		assert(Destroy != nullptr);
		Destroy(plugin_);
	}

	void Manager::ActionBegin(Plugin::ITexture* canvas, Point2D<int> point) {
		cur_tool_->ActionBegin(canvas, point.x, point.y);
	}

	void Manager::Action(Plugin::ITexture* canvas, Point2D<int> prev_point, Point2D<int> diff) {
		cur_tool_->Action(canvas, prev_point.x, prev_point.y, diff.x, diff.y);
	}

	void Manager::ActionEnd(Plugin::ITexture* canvas, Point2D<int> point) {
		cur_tool_->ActionEnd(canvas, point.x, point.y);
	}

	uint Manager::GetThickness() {
		return thickness_;
	}

	uint Manager::GetColor() {
		uint res = ::GetColor(color_);
		return res;
	}

	std::list<Plugin::ITool*>& Manager::GetToolsList() {
		return tools_;
	}

	std::list<Plugin::IFilter*>& Manager::GetFiltersList() {
		return filters_;
	}

	void Manager::SetColor(const Color& color) {
		color_ = color;
		Plugin::WidgetFactory* w_f = dynamic_cast<Plugin::WidgetFactory*>(kApi->GetWidgetFactory());
		std::vector<Plugin::Palette*> palettes = w_f->GetPalettes();
		for (auto p : palettes) {
			assert(p->callback_ != nullptr);

			uint r = (uint)color.red;
      uint g = (uint)color.green;
      uint b = (uint)color.blue;
      uint a = (uint)color.alpha;
      uint res = (r << 24) + (g << 16) + (b << 8) + a;

      printf("res = %u\n", res);
			p->callback_->RespondOnChangeColor(res);
		}
	}

	void Manager::SetCurrentTool(Plugin::ITool* tool) {
		cur_tool_ = tool;
	}

 	Pencil::Pencil()
 	: manager_(Manager::GetInstance()) {}

  void Pencil::ActionBegin(Plugin::ITexture* canvas, int x, int y) {
  	canvas->DrawCircle({x, y, manager_->GetThickness(), 0, manager_->GetColor(), 0});
  }

  void Pencil::Action(Plugin::ITexture* canvas, int x, int y, int dx, int dy) {
  	canvas->DrawLine({x, y, x + dx, y + dy, manager_->GetThickness(), manager_->GetColor()});
  }

  void Pencil::ActionEnd(Plugin::ITexture* canvas, int x, int y) {}

  const char* Pencil::GetIconFileName() const {
  	return "tool_pencil.png";
  }

  Plugin::IPreferencesPanel* Pencil::GetPreferencesPanel() const {
  	return nullptr;
  }

	Eraser::Eraser()
 	: manager_(Manager::GetInstance()) {}

  void Eraser::ActionBegin(Plugin::ITexture* canvas, int x, int y) {
  	canvas->DrawCircle({x, y, manager_->GetThickness(), 0, 0xFFFFFFFF, 0});
  }

  void Eraser::Action(Plugin::ITexture* canvas, int x, int y, int dx, int dy) {
  	canvas->DrawLine({x, y, x + dx, y + dy, manager_->GetThickness(), 0xFFFFFFFF});
  }

  void Eraser::ActionEnd(Plugin::ITexture* canvas, int x, int y) {}

  const char* Eraser::GetIconFileName() const {
  	return "tool_eraser.png";
  }

  Plugin::IPreferencesPanel* Eraser::GetPreferencesPanel() const {
  	return nullptr;
  }
}