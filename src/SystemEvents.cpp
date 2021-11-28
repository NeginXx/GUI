#include <SDL2/SDL.h>
#include "../include/SystemEvents.h"

bool IsSomeEventInQueue(SystemEvent* event) {
  SDL_Event sdl_event;
  if (!SDL_PollEvent(&sdl_event)) {
    return false;
  }

  switch (sdl_event.type) {
    case SDL_QUIT: {
      event->type = SystemEvent::kQuit;
      break;
    }

    case SDL_KEYDOWN: {
      event->type = SystemEvent::kKeyboardKeyDown;
      event->info.keyboard_key_click = {sdl_event.key.keysym.scancode};
      break;
    }

    case SDL_KEYUP: {
      event->type = SystemEvent::kKeyboardKeyUp;
      event->info.keyboard_key_click = {sdl_event.key.keysym.scancode};
      break;
    }

    case SDL_MOUSEBUTTONDOWN: {
      event->type = SystemEvent::kMouseButtonDown;
      if (sdl_event.button.button == SDL_BUTTON_LEFT) {
        event->info.mouse_click.button = MouseClickInfo::kLeftButton;
      } else {
        event->info.mouse_click.button = MouseClickInfo::kRightButton;
      }
      auto info = sdl_event.button;
      assert(info.x >= 0);
      assert(info.y >= 0);
      event->info.mouse_click.coordinate = Point2D<uint>{ (uint)info.x,
                                                          (uint)info.y };
      break;
    }

    case SDL_MOUSEBUTTONUP: {
      event->type = SystemEvent::kMouseButtonUp;
      if (sdl_event.button.button == SDL_BUTTON_LEFT) {
        event->info.mouse_click.button = MouseClickInfo::kLeftButton;
      } else {
        event->info.mouse_click.button = MouseClickInfo::kRightButton;
      }
      auto info = sdl_event.button;
      assert(info.x >= 0);
      assert(info.y >= 0);
      event->info.mouse_click.coordinate = Point2D<uint>{ (uint)info.x,
                                                          (uint)info.y };
      break;
    }

    case SDL_MOUSEMOTION: {
      event->type = SystemEvent::kMouseMotion;
      auto info = sdl_event.motion;
      assert(info.x >= 0);
      assert(info.y >= 0);
      event->info.mouse_motion = { {(uint)info.x, (uint)info.y},
                                   {(uint)Max(0, info.x + info.xrel), (uint)Max(0, info.y + info.yrel)} };
      break;
    }

    case SDL_WINDOWEVENT: {
      if (sdl_event.window.event != SDL_WINDOWEVENT_RESIZED) {
        return false;
      }

      event->type = SystemEvent::kWindowResize;
      auto info = sdl_event.window;
      assert(info.data1 >= 0);
      assert(info.data2 >= 0);
      event->info.window_resize = { (uint)info.data1,
                                    (uint)info.data2,
                                    info.windowID };
      break;
    }

    default: return false;
  }

  return true;
}