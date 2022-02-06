#include "../include/main.h"
#include "../include/Render.h"
#include "../include/GLWindow.h"
#include "../include/App.h"

Color GetColor(uint color) {
  unsigned char arr[4] = {};
  for (size_t i = 0; i < 4; ++i) {
    arr[i] = static_cast<unsigned char>(color & 0xFF);
    color >>= 8;
  }
  return *reinterpret_cast<Color*>(arr);
}

uint GetColor(Color color) {
  uint r = (uint)color.red;
  uint g = (uint)color.green;
  uint b = (uint)color.blue;
  uint a = (uint)color.alpha;
  uint res = (a << 24) + (b << 16) + (g << 8) + r;
  return res;
}

int main() {
  InitStackTrace({SIGSEGV, SIGABRT}, 100);
  srand(time(NULL));
  GLWindow window(1848, 1016);
  Render render(window);
  RunApp(&window, &render);
  FreeStackTrace();
}