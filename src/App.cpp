#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include <queue>
#include "../include/main.h"
#include "../include/Render.h"
#include "../include/GLWindow.h"
#include "../include/List.h"
#include "../include/SystemEvents.h"
#include "../include/Texture.h"
#include "../include/Window.h"
#include "../include/FunctorQueue.h"
#include "../include/GUIConstants.h"

// Declaring textures and draw functors
#define DEFINE_SKIN(Scalability, Name, file_name) \
  extern Texture* kTexture##Name;                 \
  extern DrawFunctor::Scalability##Texture* kFuncDraw##Name;
  #include "../include/DEFINE_SKIN.h"
#undef DEFINE_SKIN

// extern Widget::Canvas* CreateCanvas(Widget::Window* parent,
//                                     const Rectangle& pos,
//                                     Render* render) {
//   Widget::Window* window = CreateStandardWindow(parent, pos);
//   Texture* texture = new Texture(1848, 1016, render);
//   texture->SetBackgroundColor({0, 0, 0});
//   CanvasFunctor* canvas_func = new CanvasFunctor(render, texture, 1, 1);
//   Widget::Canvas* canvas = new Widget::Canvas({ {kResizeWidgetOfs, kResizeWidgetOfs + kStandardTitlebarHeight}, pos.width - 2 * kResizeWidgetOfs,
//                               pos.height - 2 * kResizeWidgetOfs - kStandardTitlebarHeight },
//                               canvas_func);
//   window->AddChild(canvas);
//   return canvas;
// }

class MainBar : public Widget::Container {
 public:
  MainBar() = delete;

  ~MainBar() override {
    delete func_open_file_;
  }

  MainBar(Widget::MainWindow* main_window,
          Render* render, uint width)
  : Container({{0, 0}, width, kStandardTitlebarHeight}, {}, kFuncDrawTexMain)
  {
    func_open_file_ = new Functor::OpenCanvas(main_window, render);
    auto file_button =
    new UserWidget::ButtonWithText({0, 0}, main_window, func_open_file_, {kFuncDrawTexMain, kFuncDrawTexMainDark, kFuncDrawTexMainDarkExtra},
                                   "File", render, kWhite);
    AddChild(file_button);
    main_window->AddChild(this);
  }

 private:
  Functor::OpenCanvas* func_open_file_;
};

void RunApp(GLWindow* gl_window, Render* render) {
  // Initializing textures and draw functors
  #define DEFINE_SKIN(Scalability, Name, file_name)  \
    kTexture##Name = new Texture(file_name, render); \
    kFuncDraw##Name = new DrawFunctor::Scalability##Texture(kTexture##Name);
    #include "../include/DEFINE_SKIN.h"
  #undef DEFINE_SKIN

  // Creating windows
  // -----------------------------------------------
  const uint gl_window_width = gl_window->GetWidth();
  const uint gl_window_height = gl_window->GetHeight();

  auto main_window = new Widget::MainWindow({{0, 0}, gl_window_width, gl_window_height}, {}, kFuncDrawTexMainLight);
  auto main_bar = new MainBar(main_window, render, gl_window_width);
  auto canvas = new UserWidget::PaintWindow({{100, 100}, 1000, 700}, main_window, render);
  // auto hole_window = new UserWidget::HoleWindow({{-500, kStandardTitlebarHeight}, 510, 700}, main_window, render);

  // Rectangle pos = {{500, 500}, kStandardButtonWidth * 10, kStandardButtonWidth};
  // auto text = new Texture("Can", render, kBlack);
  // auto ddd = new DrawFunctor::MultipleFunctors({kFuncDrawTexStriped, new DrawFunctor::TilingTexture(text)});
  // auto some_button = new Widget::Button(pos, main_window, nullptr, {ddd});
  // main_window->AddChild(some_button);

  // -----------------------------------------------

  SystemEvent event = {};
  bool is_running = true;
  while (is_running) {
    uint time1 = SDL_GetTicks();

    render->SetBackgroundColor(kBlack);
    main_window->Draw();
    gl_window->RenderPresent(render);

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
          printf("!\n");
          main_window->ProcessSystemEvent(event);
          break;
        }
      }
    }

    FunctorQueue& queue = FunctorQueue::GetInstance();

    while (!queue.IsEmpty()) {
      Functor::Abstract* func = queue.Pop();
      func->Action();
    }

    // DelayIfNeeded(time1, SDL_GetTicks());
    // printf("fps = %u\n", (uint)(1 / ((float)(SDL_GetTicks() - time1) / 1000)));
  }

  delete main_window;
  // deleting textures and draw functors
  #define DEFINE_SKIN(Scalability, Name, file_name) \
    delete kTexture##Name;                          \
    delete kFuncDraw##Name;
    #include "../include/DEFINE_SKIN.h"
  #undef DEFINE_SKIN
}