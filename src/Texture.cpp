#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <unistd.h>
#include "../include/Render.h"
#include "../include/Texture.h"

const char* kSkinsDirName = "skins";
const uint kTextOfs = 3;

bool IsFileExists(const char* name) {
	return access(name, F_OK) == 0;
}

Texture::Texture(const char* image_name, Render* render)
: render_(render)
{
	static char image_path[100] = {};
	sprintf(image_path, "%s/%s", kSkinsDirName, image_name);

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
	width_  = (uint)w;
	height_ = (uint)h;
}

Texture::Texture(const char* text, Render* render, const Color& color)
: render_(render)
{
	SDL_Surface* text_surface = TTF_RenderText_Solid(render_->GetFont(), text, SDL_Color{color.red, color.green, color.blue, color.alpha});
	assert(text_surface != nullptr);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(render_->render_, text_surface);
	assert(text_texture != nullptr);
	SDL_FreeSurface(text_surface);

	int w = 0;
	int h = 0;
	SDL_QueryTexture(text_texture, NULL, NULL, &w, &h);
	assert(w >= 0);
	assert(h >= 0);
	width_  = (uint)w + kTextOfs * 2;
	height_ = (uint)h + kTextOfs * 2;

	texture_ = SDL_CreateTexture(render_->render_,
		                           SDL_PIXELFORMAT_RGBA8888,
		                           SDL_TEXTUREACCESS_TARGET,
		                           width_, height_);
	SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);

	SDL_SetRenderTarget(render_->render_, texture_);
	SDL_Rect dest = {kTextOfs, kTextOfs, w, h};
  SDL_RenderCopy(render_->render_, text_texture, nullptr, &dest);
  SDL_DestroyTexture(text_texture);
}

Texture::Texture(uint width, uint height, Render* render, const Color& color)
: render_(render), width_(width), height_(height)
{
	texture_ = SDL_CreateTexture(render->render_,
		                           SDL_PIXELFORMAT_RGBA8888,
		                           SDL_TEXTUREACCESS_TARGET,
		                           width, height);
	SDL_SetTextureBlendMode(texture_, SDL_BLENDMODE_BLEND);
	SetBackgroundColor(color);
	// texture_ = SDL_CreateTexture(render->render_,
	//                              SDL_PIXELFORMAT_RGBA8888,
	//                              SDL_TEXTUREACCESS_STREAMING,
	//                              width, height);
	// texture_ = SDL_CreateTexture(render->render_,
	//                              SDL_PIXELFORMAT_RGBA8888,
	//                              SDL_TEXTUREACCESS_STATIC,
	//                              width, height);
}

Texture::Texture(const Texture& texture)
: Texture(texture.width_, texture.height_, texture.render_)
{
	CopyTexture(texture, nullptr);
}

Texture::~Texture() {
	$;
	SDL_DestroyTexture(texture_);
	texture_ = NULL;
	$$;
}

void Texture::CopyTexture(const Texture& texture, const Rectangle* dest) {
	SDL_SetRenderTarget(render_->render_, texture_);
	SDL_Rect dest_rect = {};
	SDL_Rect* dest_ptr = &dest_rect;
	if (dest != nullptr) {
		dest_rect = { dest->corner.x, dest->corner.y,
			           (int)(dest->width), (int)(dest->height) };
	} else {
		dest_ptr = nullptr;
	}
	SDL_RenderCopy(render_->render_, texture.texture_, nullptr, dest_ptr);
}

void Texture::Draw(const Rectangle* src,
                   const Rectangle* dest) {
	SDL_Rect src_rect = {};
	SDL_Rect* src_ptr = &src_rect;
	if (src != nullptr) {
		src_rect = { src->corner.x, src->corner.y,
			           (int)src->width, (int)src->height };
	} else {
		src_ptr = nullptr;
	}
	SDL_Rect dest_rect = {};
	SDL_Rect* dest_ptr = &dest_rect;
	if (dest != nullptr) {
		dest_rect = { dest->corner.x, dest->corner.y,
			            (int)dest->width, (int)dest->height };
	} else {
		dest_ptr = nullptr;
	}
	assert(texture_ != nullptr);
	SDL_SetRenderTarget(render_->render_, nullptr);
	SDL_RenderCopy(render_->render_, texture_, src_ptr, dest_ptr);
}

void Texture::DrawWithNoScale(const Rectangle* dest) {
	Rectangle src = { {0, 0}, Min(width_, dest->width),
	                  Min(height_, dest->height) };
	this->Draw(&src, dest);
}

void Texture::DrawLine(const Point2D<int>& coord1,
                       const Point2D<int>& coord2,
                       const Color& color) {
	SDL_SetRenderTarget(render_->render_, texture_);
	SDL_SetRenderDrawColor(render_->render_, color.red, color.green, color.blue, color.alpha);
	SDL_RenderDrawLine(render_->render_, coord1.x, coord1.y,
		                 coord2.x, coord2.y);
}

void Texture::DrawThickLine(const Point2D<int>& coord1,
                            const Point2D<int>& coord2,
                            uint thickness,
                            const Color& color) {
	SDL_SetRenderTarget(render_->render_, texture_);
	SDL_SetRenderDrawColor(render_->render_, color.red, color.green, color.blue, color.alpha);

	float x1 = coord1.x;
	float x2 = coord2.x;
	float y1 = coord1.y;
	float y2 = coord2.y;
  Vec2D<float> start = {x1, y1};
  Vec2D<float> vec   = {x2 - x1, y2 - y1};
  uint len = (uint)vec.GetLength();

  vec.Normalize();
  for (uint i = 0; i < len; ++i) {
    DrawCircle({(int)(start.x + i * vec.x), (int)(start.y + (float)i * vec.y)}, thickness, color);
  }
}

void Texture::DrawCircle(Point2D<int> center, uint radius,
	                       const Color& color) {
  int signed_radius = (int)radius;
  uint radius_squared = radius * radius;

  for (int x = -signed_radius; x <= signed_radius; ++x) {
    int y_lenght = sqrt(radius_squared - x * x);
    for (int y = -y_lenght; y <= y_lenght; ++y) {
    	DrawPoint(Point2D<int>{x + center.x, y + center.y}, color);
    	DrawPoint(Point2D<int>{x + center.x, -y + center.y}, color);
    }
  }
}

void Texture::DrawRect(const Rectangle& position,
                       const Color& color) {
	SDL_SetRenderTarget(render_->render_, texture_);
  SDL_SetRenderDrawColor(render_->render_, color.red, color.green, color.blue, color.alpha);
	SDL_Rect dst = {position.corner.x, position.corner.y, (int)position.width, (int)position.height};
	SDL_RenderFillRect(render_->render_, &dst);
}

void Texture::DrawPoint(const Point2D<int>& coord,
                        const Color& color) {
	SDL_SetRenderTarget(render_->render_, texture_);
	SDL_SetRenderDrawColor(render_->render_, color.red, color.green, color.blue, color.alpha);
	SDL_RenderDrawPoint(render_->render_, coord.x, coord.y);
}

void Texture::DrawText(const char* text_str,
	                     const Point2D<int>& dest,
	                     const Color& color) {
	SDL_Surface* text = TTF_RenderText_Solid(render_->GetFont(), text_str, SDL_Color{color.red, color.green, color.blue, color.alpha});
	assert(text != nullptr);
	SDL_Texture* text_texture = SDL_CreateTextureFromSurface(render_->render_, text);
	assert(text_texture != nullptr);
	SDL_FreeSurface(text);
  SDL_Rect sdl_dest = {dest.x, dest.y, text->w, text->h};
	SDL_SetRenderTarget(render_->render_, texture_);
  SDL_RenderCopy(render_->render_, text_texture, nullptr, &sdl_dest);
  SDL_DestroyTexture(text_texture);
}

void Texture::SetBackgroundColor(const Color& color) {
	SDL_SetRenderTarget(render_->render_, texture_);
  SDL_SetRenderDrawColor(render_->render_, color.red, color.green, color.blue, color.alpha);
  SDL_RenderFillRect(render_->render_, nullptr);
}

uint Texture::GetWidth() const {
	return width_;
}

uint Texture::GetHeight() const {
	return height_;
}

void Texture::SaveToPNG(const char* file_name) {
  int width, height;
  SDL_QueryTexture(texture_, NULL, NULL, &width, &height);
  SDL_Surface* surface = SDL_CreateRGBSurface(0, width, height, 32, 0xff000000, 0xff0000, 0xff00, 0xff);
  SDL_SetRenderTarget(render_->render_, texture_);
  SDL_RenderReadPixels(render_->render_, NULL, surface->format->format, surface->pixels, surface->pitch);
  char file_name_png[100] = {};
  sprintf(file_name_png, "%s.png", file_name);
  IMG_SavePNG(surface, file_name_png);
  SDL_FreeSurface(surface);
}