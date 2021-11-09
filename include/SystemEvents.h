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
  Point2D<size_t> coordinate = {0, 0};
  Button button = kLeftButton;
};

struct MouseMotionInfo {
  Point2D<size_t> old_mouse_pos = {0, 0};
  Point2D<size_t> new_mouse_pos = {0, 0};
};

struct WindowResizeInfo {
  size_t new_width = 0;
  size_t new_height = 0;
  size_t window_id = 0;
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