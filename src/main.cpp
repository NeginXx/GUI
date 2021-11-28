#include "../include/main.h"
#include "../include/Render.h"
#include "../include/GLWindow.h"
#include "../include/App.h"

Color GetColor(uint color) {
  unsigned char arr[4] = {};
  for (size_t i = 0; i < 4; ++i) {
    arr[3 - i] = static_cast<unsigned char>(color & 0xFF);
    color >>= 16;
  }
  return *reinterpret_cast<Color*>(arr);
}

uint GetColor(Color color) {
  uint r = (uint)color.red;
  uint g = (uint)color.green;
  uint b = (uint)color.blue;
  uint a = (uint)color.alpha;
  return (r << 24) + (g << 16) + (b << 8) + a;
}

int main() {
  InitStackTrace({SIGSEGV, SIGABRT}, 100);
  srand(time(NULL));
  GLWindow window(1848, 1016);
  Render render(window);
  RunApp(&window, &render);
  FreeStackTrace();
}