#include <SDL2/SDL_ttf.h>
#include "../include/Render.h"
#include "../include/GLWindow.h"

const char* kFontName = "fonts/OpenSans-Bold.ttf";
const size_t kFontSize = 30;

Render::Render(const GLWindow& window) {
	assert(TTF_Init() >= 0);
	font_ = TTF_OpenFont(kFontName, kFontSize);
	assert(font_ != nullptr);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	render_ = SDL_CreateRenderer(window.GetWindow(), -1,
		                           SDL_RENDERER_ACCELERATED);
	assert(render_ != nullptr);
}

SDL_Renderer* Render::GetRender() const {
  return render_;
}

_TTF_Font* Render::GetFont() const {
	return font_;
}

void Render::DrawText(const char* text_str,
	                    const Point2D<size_t>& dest_coord,
	                    const Color& color) {
	SDL_Surface* text = TTF_RenderText_Solid(font_, text_str, SDL_Color{color.red, color.green, color.blue, color.alpha});
	assert(text != nullptr);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(render_, text);
	assert(text_texture != nullptr);
	SDL_FreeSurface(text);
  SDL_Rect dest = {(int)dest_coord.x, (int)dest_coord.y, text->w, text->h};
	SDL_SetRenderTarget(render_, nullptr);
	SDL_RenderCopy(render_, text_texture, nullptr, &dest);
  SDL_DestroyTexture(text_texture);
}

void Render::SetBackgroundColor(const Color& color) {
	SDL_SetRenderTarget(render_, nullptr);
  SDL_SetRenderDrawColor(render_, color.red, color.green, color.blue, color.alpha);
  SDL_RenderClear(render_);
}

SDL_Texture* Render::CreateTextureFromSurface(SDL_Surface* surface) {
	return SDL_CreateTextureFromSurface(render_, surface);
}

Render::~Render() {
	SDL_DestroyRenderer(render_);
}