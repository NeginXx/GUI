#include "../include/main.h"
#include "../include/Render.h"
#include "../include/GLWindow.h"
#include "../include/App.h"

#include <stdio.h>
int main() {
  InitStackTrace({SIGSEGV, SIGABRT}, 100);
  srand(time(NULL));
  volatile char* buf = new char[100];
  GLWindow window(1848, 1016);
  Render render(window);
  RunApp(&window, &render);
  FreeStackTrace();
}