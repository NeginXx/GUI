#pragma once
#include "main.h"

struct KeyboardKeyClickInfo {
  int scancode = 0;
};

struct MouseClickInfo {
  enum Button {
    kLeftButton,
    kRightButton
  };
  Point2D<uint> coordinate = {0, 0};
  Button button = kLeftButton;
};

struct MouseMotionInfo {
  Point2D<uint> old_mouse_pos = {0, 0};
  Point2D<uint> new_mouse_pos = {0, 0};
};

struct WindowResizeInfo {
  uint new_width = 0;
  uint new_height = 0;
  uint window_id = 0;
};

struct SystemEvent {
  enum Type {
    kQuit,
    kKeyboardKeyUp,
    kKeyboardKeyDown,
    kMouseButtonUp,
    kMouseButtonDown,
    kMouseMotion,
    kWindowResize,
    kUndefined
  };

  union Info {
    KeyboardKeyClickInfo keyboard_key_click;
    WindowResizeInfo     window_resize;
    MouseMotionInfo      mouse_motion;
    MouseClickInfo       mouse_click;
  };

  Type type;
  Info info;
};

bool IsSomeEventInQueue(SystemEvent* event);