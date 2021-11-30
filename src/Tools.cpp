#include <stdio.h>
#include <dlfcn.h>
#include "../include/Tools.h"
#include "../include/GUIConstants.h"

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
		sprintf(temp, "%s/%s", kPluginsDirName, "Plugin.so");
		plugin_lib_ = dlopen(temp, RTLD_NOW);
		assert(plugin_lib_ != nullptr);
		Plugin::CreateFunction Create = (Plugin::CreateFunction)dlsym(plugin_lib_, "Create");
		assert(Create != nullptr);
		plugin_ = Create(nullptr);
		tools_ = plugin_->GetTools();
		tools_.push_front(new Eraser());
		tools_.push_front(new Pencil());
		cur_tool_ = *tools_.begin();
	}

	Manager::~Manager() {
		$;
		for (auto t : tools_) delete t;
		Plugin::DestroyFunction Destroy = (Plugin::DestroyFunction)dlsym(plugin_lib_, "Destroy");
		assert(Destroy != nullptr);
		Destroy(plugin_);
		$$;
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

	void Manager::SetColor(const Color& color) {
		color_ = color;
	}

	void Manager::SetCurrentTool(Plugin::ITool* tool) {
		cur_tool_ = tool;
	}

 	Pencil::Pencil()
 	: manager_(Manager::GetInstance()) {}

  void Pencil::ActionBegin(Plugin::ITexture* canvas, int x, int y) {
  	canvas->DrawCircle(x, y, manager_->GetThickness(), manager_->GetColor());
  }

  void Pencil::Action(Plugin::ITexture* canvas, int x, int y, int dx, int dy) {
  	canvas->DrawThickLine(x, y, x + dx, y + dy, manager_->GetThickness(), manager_->GetColor());
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
  	canvas->DrawCircle(x, y, manager_->GetThickness(), 0);
  }

  void Eraser::Action(Plugin::ITexture* canvas, int x, int y, int dx, int dy) {
  	canvas->DrawThickLine(x, y, x + dx, y + dy, manager_->GetThickness(), 0xFFFFFFFF);
  }

  void Eraser::ActionEnd(Plugin::ITexture* canvas, int x, int y) {}

  const char* Eraser::GetIconFileName() const {
  	return "tool_eraser.png";
  }

  Plugin::IPreferencesPanel* Eraser::GetPreferencesPanel() const {
  	return nullptr;
  }
}