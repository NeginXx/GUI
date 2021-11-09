#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include "../include/Render.h"
#include "../include/Texture.h"

const char* image_extension = ".png";
const char* skins_file_name = "skins";

bool IsFileExists(const char* name) {
	return access(name, F_OK) == 0;
}

Texture::Texture(const char* image_name, Render* render)
: render_(render)
{
	static char image_path[100] = {};
	sprintf(image_path, "%s/%s%s", skins_file_name, image_name, image_extension);

	if (!IsFileExists(image_path)) {
		printf("ERROR: can't find file %s\nAborted\n", image_path);
		exit(255);
	}
	SDL_Surface* image_surface = IMG_Load(image_path);
	assert(image_surface != NULL);
	texture_ = render->CreateTextureFromSurface(image_surface);
	assert(texture_ != NULL);
	SDL_FreeSurface(image_surface);

	int w = 0;
	int h = 0;
	SDL_QueryTexture(texture_, NULL, NULL, &w, &h);
	assert(w >= 0);
	assert(h >= 0);
	width_  = (size_t)w;
	height_ = (size_t)h;
}

Texture::Texture(size_t width, size_t height, Render* render)
: render_(render), width_(width), height_(height)
{
	texture_ = SDL_CreateTexture(render->GetRender(),
		                           SDL_PIXELFORMAT_RGBA8888,
		                           SDL_TEXTUREACCESS_TARGET,
		                           width, height);
}

Texture::~Texture() {	
	SDL_DestroyTexture(texture_);
	texture_ = NULL;
}

void Texture::Draw(const Rectangle<size_t>* src,
                   const Rectangle<size_t>* dst) {
	SDL_Rect src_rect = {};
	SDL_Rect* src_ptr = &src_rect;
	if (src != nullptr) {
		src_rect = {(int)src->corner.x, (int)src->corner.y, (int)src->width, (int)src->height};
	} else {
		src_ptr = nullptr;
	}
	SDL_Rect dst_rect = {};
	SDL_Rect* dst_ptr = &dst_rect;
	if (dst != nullptr) {
		dst_rect = {(int)dst->corner.x, (int)dst->corner.y, (int)dst->width, (int)dst->height};
	} else {
		dst_ptr = nullptr;
	}
	assert(texture_ != nullptr);
	SDL_SetRenderTarget(render_->GetRender(), texture_);
	SDL_RenderCopy(render_->GetRender(), texture_, src_ptr, dst_ptr);
}

void Texture::DrawFittablePart(const Rectangle<size_t>* dst) {
	Rectangle<size_t> src = {{0, 0}, Min(width_, dst->width),
	                                 Min(height_, dst->height)};
	this->Draw(&src, dst);
}

void Texture::DrawLine(const Point2D<size_t>& coordinates1,
                       const Point2D<size_t>& coordinates2,
                       const Color& color) {
	SDL_SetRenderTarget(render_->GetRender(), texture_);
	SDL_SetRenderDrawColor(render_->GetRender(), color.red, color.green, color.blue, color.alpha);
	SDL_RenderDrawLine(render_->GetRender(), coordinates1.x, coordinates1.y,
		                 coordinates2.x, coordinates2.y);
}

void Texture::DrawText(const char* text_str,
	                     const Point2D<size_t>& dest_coord,
	                     const Color& color) {
	SDL_Surface* text = TTF_RenderText_Solid(render_->GetFont(), text_str, SDL_Color{color.red, color.green, color.blue, color.alpha});
	assert(text != nullptr);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(render_->GetRender(), text);
	assert(text_texture != nullptr);
	SDL_FreeSurface(text);
  SDL_Rect dest = {(int)dest_coord.x, (int)dest_coord.y, text->w, text->h};
	SDL_SetRenderTarget(render_->GetRender(), texture_);
  SDL_RenderCopy(render_->GetRender(), text_texture, nullptr, &dest);
}

void Texture::SetBackgroundColor(const Color& color) {
	SDL_SetRenderTarget(render_->GetRender(), texture_);
  SDL_SetRenderDrawColor(render_->GetRender(), color.red, color.green, color.blue, color.alpha);
  SDL_RenderClear(render_->GetRender());
}

size_t Texture::GetWidth() {
	return width_;
}

size_t Texture::GetHeight() {
	return height_;
}