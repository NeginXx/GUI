#pragma once
#include "main.h"
#include "IPlugin.h"

namespace Tool {
	class Manager {
	 public:
	  static Manager* GetInstance();
	  void ActionBegin(Plugin::ITexture* canvas, Point2D<int> point);
    void Action(Plugin::ITexture* canvas, Point2D<int> prev_point, Point2D<int> diff);
    void ActionEnd(Plugin::ITexture* canvas, Point2D<int> point);
	  uint GetThickness();
	  uint GetColor();
	  std::list<Plugin::ITool*>& GetToolsList();
	  std::list<Plugin::IFilter*>& GetFiltersList();
	  void SetColor(const Color& color);
	  void SetCurrentTool(Plugin::ITool* tool);
  	~Manager();

	 private:
	 	uint thickness_;
	 	Color color_;
	 	std::list<Plugin::ITool*> tools_;
	 	std::list<Plugin::IFilter*> filters_;
	 	Plugin::ITool* cur_tool_;
	 	void* plugin_lib_;
	 	Plugin::IPlugin* plugin_;

	  Manager();
	  void Init();
	  Manager(const Manager&) = delete;
	  Manager& operator=(const Manager&) = delete;
	  Manager(Manager&&) = delete;
	  Manager& operator=(Manager&&) = delete;
	};

	class Pencil : public Plugin::ITool {
	 public:
	 	Pencil();

	  void ActionBegin(Plugin::ITexture* canvas, int x, int y) override;
	  void Action(Plugin::ITexture* canvas, int x, int y, int dx, int dy) override;
	  void ActionEnd(Plugin::ITexture* canvas, int x, int y) override;
	  const char* GetIconFileName() const override;
	  Plugin::IPreferencesPanel* GetPreferencesPanel()const override;

	 private:
	 	Manager* manager_;
	};

	class Eraser : public Plugin::ITool {
	 public:
	 	Eraser();

	  void ActionBegin(Plugin::ITexture* canvas, int x, int y) override;
	  void Action(Plugin::ITexture* canvas, int x, int y, int dx, int dy) override;
	  void ActionEnd(Plugin::ITexture* canvas, int x, int y) override;
	  const char* GetIconFileName() const override;
	  Plugin::IPreferencesPanel* GetPreferencesPanel()const override;

	 private:
	 	Manager* manager_;
	};
}