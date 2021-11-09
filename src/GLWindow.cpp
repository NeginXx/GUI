#include <SDL2/SDL_ttf.h>
#include "../include/Render.h"
#include "../include/GLWindow.h"

GLWindow::GLWindow(size_t width, size_t height) {
	window_ = SDL_CreateWindow("Unnamed", SDL_WINDOWPOS_UNDEFINED,
	                           SDL_WINDOWPOS_UNDEFINED, width, height,
	                           SDL_WINDOW_RESIZABLE);
  assert(window_ != NULL);
}

size_t GLWindow::GetWidth() const {
	int w = 0;
	SDL_GetWindowSize(window_, &w, nullptr);
	assert(w >= 0);
	return (size_t)w;
}

size_t GLWindow::GetHeight() const {
	int h = 0;
	SDL_GetWindowSize(window_, nullptr, &h);
	assert(h >= 0);
	return (size_t)h;
}

SDL_Window* GLWindow::GetWindow() const {
	return window_;
}

void GLWindow::RenderPresent(Render* render) {
 	SDL_RenderPresent(render->GetRender());
}

GLWindow::~GLWindow() {
	SDL_DestroyWindow(window_);
}