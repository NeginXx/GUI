#pragma once
#include "main.h"

class Texture;
class GLWindow;
class SDL_Renderer;
class SDL_Texture;
class SDL_Surface;
class _TTF_Font;

namespace Plugin {
  class Texture;
}

class Render {
 public:
 	Render(const GLWindow& window);
 	SDL_Renderer* GetRender() const;
 	_TTF_Font* GetFont() const;
  void DrawText(const char* text_str,
  	            const Point2D<int>& dest_coord,
  	            const Color& color = {});
  void DrawPoint(const Point2D<int>& coord,
                 const Color& color = {});
	void SetBackgroundColor(const Color& color);
	SDL_Texture* CreateTextureFromSurface(SDL_Surface* surface);
  ~Render();

  friend class Texture;
  friend class Plugin::Texture;

 private:
 	SDL_Renderer* render_ = nullptr;
	_TTF_Font* font_ = nullptr;
};