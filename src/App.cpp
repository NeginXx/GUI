#include <SDL2/SDL.h>
#include <math.h>
#include <time.h>
#include "../include/main.h"
#include "../include/Render.h"
#include "../include/GLWindow.h"
#include "../include/SystemEvents.h"
#include "../include/Texture.h"
#include "../include/Widget.h"
#include "../include/DropdownList.h"
#include "../include/Plugin.h"
#include "../include/Canvas.h"

// Declaring textures and draw functors
#define DEFINE_SKIN(Scalability, Name, file_name)         \
  Texture* kTexture##Name;                                \
  DrawFunctor::Scalability##Texture* kFuncDraw##Name;     \
  DrawFunctor::MultipleFunctors* kFuncDraw##Name##Framed; \
  DrawFunctor::Scalability##Texture* kFuncDraw##Name##Auxiliary;
  #include "../include/DEFINE_SKIN.h"
#undef DEFINE_SKIN

class MainBar : public Widget::Container {
 public:
  MainBar() = delete;

  ~MainBar() override {
    delete dropdown_list_;
    delete func_;
    delete func_open_hole_window_;
    delete func_open_canvas_;
  }

  MainBar(Widget::MainWindow* main_window,
          Render* render, uint width)
  : Container({{0, 0}, width, kStandardTitlebarHeight}, {}, kFuncDrawTexMain)
  {
    func_open_canvas_ = new Functor::OpenCanvas(main_window, render);
    func_open_hole_window_ = new Functor::OpenHoleWindow(main_window, render);
    // auto file_button =
    // new UserWidget::BasicButtonWithText({0, 0}, main_window, func_open_canvas_, {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra},
    //                                     render, kWhite}, "File");
    // AddChild(file_button);
    // int x = file_button->GetPosition().width;

    func_ = new Functor::DropdownListPopUp(nullptr);
    auto some_button =
    new UserWidget::ButtonOnPressWithText({0, 0}, main_window, func_, {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra},
                                          render, kWhite}, "File");
    dropdown_list_ =
    new UserWidget::DropdownList(main_window, main_window,
                                 some_button, 200, kStandardTitlebarHeight, {{func_open_canvas_, "Open canvas"}, {func_open_hole_window_, "Open hole window"}},
                                 {{kFuncDrawTexMainFramed, kFuncDrawTexMainDarkFramed, kFuncDrawTexMainDarkExtra}, render, kWhite});
    func_->SetDropdownList(dropdown_list_);
    AddChild(some_button);

    main_window->AddChild(this);
  }

 private:
  Functor::OpenCanvas* func_open_canvas_;
  Functor::OpenHoleWindow* func_open_hole_window_;
  Functor::DropdownListPopUp* func_;
  UserWidget::DropdownList* dropdown_list_;
};

Plugin::API* kApi;
MainBar* kMainBar;

void RunApp(GLWindow* gl_window, Render* render) {
  // Initializing textures and draw functors
  Texture* kTextureFrame = new Texture("tex_black.png", render);
  DrawFunctor::Abstract* kFuncDrawFrame = new DrawFunctor::ScalableTexture(kTextureFrame);
  #define DEFINE_SKIN(Scalability, Name, file_name)  \
    kTexture##Name = new Texture(file_name, render); \
    kFuncDraw##Name = new DrawFunctor::Scalability##Texture(kTexture##Name); \
    kFuncDraw##Name##Auxiliary = new DrawFunctor::Scalability##Texture(kTexture##Name, {kStandardFrameWidth, kStandardFrameWidth}); \
    kFuncDraw##Name##Framed = new DrawFunctor::MultipleFunctors({kFuncDrawFrame, kFuncDraw##Name##Auxiliary})
    #include "../include/DEFINE_SKIN.h"
  #undef DEFINE_SKIN

  // Creating windows
  // -----------------------------------------------
  const uint gl_window_width = gl_window->GetWidth();
  const uint gl_window_height = gl_window->GetHeight();

  auto main_window = new Widget::MainWindow({{0, 0}, gl_window_width, gl_window_height}, {}, kFuncDrawTexMainLight);
  kApi = new Plugin::API(main_window, render);
  kMainBar = new MainBar(main_window, render, gl_window_width);
  // auto canvas = new UserWidget::PaintWindow({{100, 100}, 1000, 700}, main_window, render);
  // auto hole_window = new UserWidget::HoleWindow({{-500, kStandardTitlebarHeight}, 510, 700}, main_window, render);

  // Rectangle pos = {{500, 500}, kStandardButtonWidth * 10, kStandardButtonWidth};
  // auto text = new Texture("Can", render, kBlack);
  // auto ddd = new DrawFunctor::MultipleFunctors({kFuncDrawTexStriped, new DrawFunctor::TilingTexture(text)});
  // auto some_button = new Widget::Button(pos, main_window, nullptr, {ddd});
  // main_window->AddChild(some_button);

  // -----------------------------------------------

  // Texture tex(1920, 1080, render, kBlack);
  // tex.SaveToPNG("tex_black");


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
    // SDL_Delay(200);

    // DelayIfNeeded(time1, SDL_GetTicks());
    // printf("fps = %u\n", (uint)(1 / ((float)(SDL_GetTicks() - time1) / 1000)));
  }

  delete main_window;
  delete Tool::Manager::GetInstance();
  // deleting textures and draw functors
  delete kTextureFrame;
  delete kFuncDrawFrame;
  #define DEFINE_SKIN(Scalability, Name, file_name) \
    delete kTexture##Name;                          \
    delete kFuncDraw##Name;                         \
    delete kFuncDraw##Name##Auxiliary;              \
    delete kFuncDraw##Name##Framed;
    #include "../include/DEFINE_SKIN.h"
  #undef DEFINE_SKIN
}

// 0x6080001ddea0