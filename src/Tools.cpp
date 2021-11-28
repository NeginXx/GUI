#include "../include/Tools.h"

namespace Tool {
	Manager& Manager::GetInstance() {
		static Manager instance;
	  return instance;
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
		return ::GetColor(color_);
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

	Manager::~Manager() {
		for (auto t : tools_) delete t;
	}

  Manager::Manager()
  : thickness_(3),
    color_(kBlack),
    tools_({new Pencil(), new Eraser()}) {}

 	Pencil::Pencil()
 	: manager_(Manager::GetInstance()) {}

  void Pencil::ActionBegin(Plugin::ITexture* canvas, int x, int y) {
  	canvas->DrawCircle(x, y, manager_.GetThickness(), manager_.GetColor());
  }

  void Pencil::Action(Plugin::ITexture* canvas, int x, int y, int dx, int dy) {
  	canvas->DrawThickLine(x, y, x + dx, y + dy, manager_.GetThickness(), manager_.GetColor());
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
  	canvas->DrawCircle(x, y, manager_.GetThickness(), 0);
  }

  void Eraser::Action(Plugin::ITexture* canvas, int x, int y, int dx, int dy) {
  	canvas->DrawThickLine(x, y, x + dx, y + dy, manager_.GetThickness(), 0);
  }

  void Eraser::ActionEnd(Plugin::ITexture* canvas, int x, int y) {}

  const char* Eraser::GetIconFileName() const {
  	return "tool_eraser.png";
  }

  Plugin::IPreferencesPanel* Eraser::GetPreferencesPanel() const {
  	return nullptr;
  }
}