#include "main.h"

namespace Tool {
	enum Type {
		kPencil,
		kEraser
	};

	struct Pencil {
	 	Color color = {};
	 	uint thickness = 3;
	};

	struct Eraser {
		uint thickness = 3;
	};

	class Manager {
	 public:
	  static Manager& GetInstance() {
	  	static Manager instance;
	    return instance;
	  }

	  Tool::Type GetCurTool() {
	  	return cur_tool_;
	  }

	  void SetCurTool(Tool::Type cur_tool) {
	  	cur_tool_ = cur_tool;
	  }

	  Pencil& GetPencil() {
	  	return pencil_;
	  }

	  Eraser& GetEraser() {
	  	return eraser_;
	  }

  	~Manager() = default;

	 private:
	 	Tool::Type cur_tool_ = kPencil;
	 	Pencil pencil_ = {};
	 	Eraser eraser_ = {};

	  Manager() = default;

	  Manager(const Manager&) = delete;
	  Manager& operator=(const Manager&) = delete;
	  Manager(Manager&&) = delete;
	  Manager& operator=(Manager&&) = delete;
	};
}