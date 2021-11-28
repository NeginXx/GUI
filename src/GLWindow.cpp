#include <SDL2/SDL_ttf.h>
#include "../include/Render.h"
#include "../include/GLWindow.h"

GLWindow::GLWindow(uint width, uint height) {
	window_ = SDL_CreateWindow("Unnamed", SDL_WINDOWPOS_UNDEFINED,
	                           SDL_WINDOWPOS_UNDEFINED, width, height,
	                           SDL_WINDOW_RESIZABLE);
  assert(window_ != NULL);
}

uint GLWindow::GetWidth() const {
	int w = 0;
	SDL_GetWindowSize(window_, &w, nullptr);
	assert(w >= 0);
	return (uint)w;
}

uint GLWindow::GetHeight() const {
	int h = 0;
	SDL_GetWindowSize(window_, nullptr, &h);
	assert(h >= 0);
	return (uint)h;
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