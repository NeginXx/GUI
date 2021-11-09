#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include "../include/main.h"
#include "../include/Render.h"
#include "../include/GLWindow.h"
#include "../include/List.h"
#include "../include/SystemEvents.h"
#include "../include/Texture.h"
#include "../include/Window.h"

#define MODE 2

const size_t max_fps = 100;
const struct Color kLightPurple = {171, 60, 255},
                   kLightGreen  = {107, 216, 79},
                   kLightPink   = {255, 153, 204},
                   kBlue        = {0, 0, 255},
                   kRed         = {255, 0, 0},
                   kBlack       = {0, 0, 0},
                   kWhite       = {255, 255, 255};

const size_t kResizeWidgetOfs = 3;
const size_t kStandardHeight = 25;
const size_t kStandardButtonWidth = 21;

namespace Global {
  #define DEFINE_TEXTURE(name) Texture* texture_##name = nullptr
    DEFINE_TEXTURE(button_close);
    DEFINE_TEXTURE(button_hide);
    DEFINE_TEXTURE(icon_folder);
    DEFINE_TEXTURE(icon_warning);
    DEFINE_TEXTURE(texture_main_light);
    DEFINE_TEXTURE(texture_main);
    DEFINE_TEXTURE(texture_striped);
    DEFINE_TEXTURE(texture_striped_light);
    DEFINE_TEXTURE(texture_white);
    DEFINE_TEXTURE(tool_eraser);
    DEFINE_TEXTURE(tool_pencil);
  #undef DEFINE_TEXTURE

  #define DEFINE_DRAW_FUNCTOR(Type, name) \
    DrawFunctor::WindowWith##Type##Texture* func_draw_##name = nullptr
    DEFINE_DRAW_FUNCTOR(Scalable, button_close);
    DEFINE_DRAW_FUNCTOR(Scalable, button_hide);
    DEFINE_DRAW_FUNCTOR(Scalable, icon_folder);
    DEFINE_DRAW_FUNCTOR(Scalable, icon_warning);
    DEFINE_DRAW_FUNCTOR(Chunked,  texture_main_light);
    DEFINE_DRAW_FUNCTOR(Chunked,  texture_main);
    DEFINE_DRAW_FUNCTOR(Chunked,  texture_striped);
    DEFINE_DRAW_FUNCTOR(Chunked,  texture_striped_light);
    DEFINE_DRAW_FUNCTOR(Chunked,  texture_white);
    DEFINE_DRAW_FUNCTOR(Scalable, tool_eraser);
    DEFINE_DRAW_FUNCTOR(Scalable, tool_pencil);
  #undef DEFINE_DRAW_FUNCTOR
}

extern Widget::Window* CreateStandardWindow(Widget::Window* parent,
                                            const Rectangle<size_t>& pos) {
  ActionFunctor::CloseButton* func_close_button = new ActionFunctor::CloseButton(nullptr, nullptr);
  MoveFunctor::TitleBar* func_title_bar = new MoveFunctor::TitleBar(nullptr);
  ResizeFunctor::WindowBasic* func_resize = new ResizeFunctor::WindowBasic(nullptr);

  const size_t title_bar_width = pos.width - 2 * kResizeWidgetOfs;
  assert(kStandardHeight >= kStandardButtonWidth);
  const size_t button_ofs = (kStandardHeight - kStandardButtonWidth) / 2;
  Widget::Button* button_close =
  new Widget::Button({{title_bar_width - kStandardButtonWidth - button_ofs, button_ofs}, kStandardButtonWidth , kStandardButtonWidth},
                     func_close_button, Global::func_draw_button_close);

  Widget::TitleBar* title_bar =
  new Widget::TitleBar({{kResizeWidgetOfs, kResizeWidgetOfs}, title_bar_width, kStandardHeight},
                       {button_close}, func_title_bar, Global::func_draw_texture_striped);

  Widget::Resize* resize_widget =
  new Widget::Resize({{0, 0}, pos.width, pos.height}, func_resize, Global::func_draw_texture_white);

  Widget::Window* background =
  new Widget::Window({ {kResizeWidgetOfs, kResizeWidgetOfs + kStandardHeight}, pos.width - 2 * kResizeWidgetOfs,
                       pos.height - 2 * kResizeWidgetOfs - kStandardHeight },
                     {}, Global::func_draw_texture_striped);

  Widget::Window* window =
  new Widget::Window(pos, {title_bar, background, resize_widget}, nullptr);

  parent->AddChild(window);
  func_title_bar->SetWidgetToMove(window);
  func_close_button->SetWidgetToClose(window);
  func_close_button->SetWindowParent (parent);

  return window;
}

extern Widget::Canvas* CreateCanvas(Widget::Window* parent,
                                    const Rectangle<size_t>& pos,
                                    Render* render) {
  Widget::Window* window = CreateStandardWindow(parent, pos);
  Texture* texture = new Texture(1848, 1016, render);
  texture->SetBackgroundColor({0, 0, 0});
  CanvasFunctor* canvas_func = new CanvasFunctor(render, texture, 1, 1);
  Widget::Canvas* canvas = new Widget::Canvas({ {kResizeWidgetOfs, kResizeWidgetOfs + kStandardHeight}, pos.width - 2 * kResizeWidgetOfs,
                              pos.height - 2 * kResizeWidgetOfs - kStandardHeight },
                              canvas_func);
  window->AddChild(canvas);
  return canvas;
}

void RunApp(GLWindow* gl_window, Render* render) {
  #define CREATE_TEXTURE(name) Global::texture_##name = new Texture(#name, render)
    CREATE_TEXTURE(button_close);
    CREATE_TEXTURE(button_hide);
    CREATE_TEXTURE(icon_folder);
    CREATE_TEXTURE(icon_warning);
    CREATE_TEXTURE(texture_main_light);
    CREATE_TEXTURE(texture_main);
    CREATE_TEXTURE(texture_striped);
    CREATE_TEXTURE(texture_striped_light);
    CREATE_TEXTURE(texture_white);
    CREATE_TEXTURE(tool_eraser);
    CREATE_TEXTURE(tool_pencil);
  #undef CREATE_TEXTURE

  #define CREATE_DRAW_FUNCTOR(Type, name) \
    Global::func_draw_##name = new DrawFunctor::WindowWith##Type##Texture(Global::texture_##name)
    CREATE_DRAW_FUNCTOR(Scalable, button_close);
    CREATE_DRAW_FUNCTOR(Scalable, button_hide);
    CREATE_DRAW_FUNCTOR(Scalable, icon_folder);
    CREATE_DRAW_FUNCTOR(Scalable, icon_warning);
    CREATE_DRAW_FUNCTOR(Chunked,  texture_main_light);
    CREATE_DRAW_FUNCTOR(Chunked,  texture_main);
    CREATE_DRAW_FUNCTOR(Chunked,  texture_striped);
    CREATE_DRAW_FUNCTOR(Chunked,  texture_striped_light);
    CREATE_DRAW_FUNCTOR(Chunked,  texture_white);
    CREATE_DRAW_FUNCTOR(Scalable, tool_eraser);
    CREATE_DRAW_FUNCTOR(Scalable, tool_pencil);
  #undef CREATE_DRAW_FUNCTOR

  Texture texture_button_file("texture_white", render);

  // Creating functors
  // -----------------------------------------------
  DrawFunctor::WindowWithScalableTexture func_draw_texture_button_file(&texture_button_file);
  ActionFunctor::FileButton  func_file_button  (nullptr);
  // -----------------------------------------------

  // Creating windows
  // -----------------------------------------------
  const size_t width = gl_window->GetWidth();
  const size_t height = gl_window->GetHeight();

  Rectangle<size_t> pos = {{0, 0}, kStandardHeight, kStandardHeight};
  Widget::Button* button_file =
    new Widget::Button(pos, &func_file_button, &func_draw_texture_button_file);

  pos = {{0, 0}, width, kStandardHeight};
  Widget::TitleBar* main_menu_bar =
    new Widget::TitleBar(pos, {button_file}, nullptr, Global::func_draw_texture_main_light);

  Widget::Window* main_window = new Widget::Window({{0, 0}, width, height}, {main_menu_bar}, Global::func_draw_texture_main);

  Widget::Window* window1 =
  CreateStandardWindow(main_window, {{100, 100}, 300, 300});

  // Widget::Window* window2 =
  // CreateStandardWindow(main_window, {{400, 400}, 300, 200});

  Widget::Canvas* canvas = CreateCanvas(main_window, {{400, 400}, 300, 200}, render);
  // -----------------------------------------------

  // Setting up functors
  // -----------------------------------------------
  func_file_button.SetParent(main_window);
  // -----------------------------------------------

  bool is_mouse_pressed = false;
  SystemEvent event = {};
  bool is_running = true;
  while (is_running) {
    size_t time1 = SDL_GetTicks();

    render->SetBackgroundColor(kWhite);
    main_window->Draw({0, 0});
    // render.DrawText("File", {2, 0}, kWhite);
    gl_window->RenderPresent(render);
    // SDL_Delay(20000);

    event.type = SystemEvent::kUndefined;
    while (IsSomeEventInQueue(&event)) {
      switch (event.type) {
        case SystemEvent::kUndefined: {
          assert("BUG");
          break;
        }

        case SystemEvent::kQuit: {
          is_running = false;
          break;
        }

        default: {
          main_window->ProcessSystemEvent({0, 0}, event);
          break;
        }
      }
    }

    // DelayIfNeeded(time1, SDL_GetTicks());
  }

  delete main_window;
}