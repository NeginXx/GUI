#include "../include/main.h"
#include "../include/Render.h"
#include "../include/GLWindow.h"
#include "../include/App.h"

int main() {
  signal(SIGSEGV, PrintStackInfoAndExit);
  srand(time(NULL));
  GLWindow window(1848, 1016);
  // GLWindow window(1000, 1000);
  Render render(window);
  RunApp(&window, &render);
}