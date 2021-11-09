#pragma once
class Render;
class SDL_Texture;

class Texture {
 public:
 	Texture(const char* image_name, Render* render);
  Texture(size_t width, size_t height, Render* render);
 	~Texture();
  void Draw(const Rectangle<size_t>* src,
            const Rectangle<size_t>* dst);
  void DrawFittablePart(const Rectangle<size_t>* dst);
  void DrawLine(const Point2D<size_t>& coordinates1,
                const Point2D<size_t>& coordinates2,
                const Color& color = {});
  void DrawText(const char* text_str,
                const Point2D<size_t>& dest_coord,
                const Color& color);
  void SetBackgroundColor(const Color& color);
  size_t GetWidth() const;
  size_t GetHeight() const;
 	friend class Render;

 private:
 	SDL_Texture* texture_;
  Render* render_;
  size_t width_;
  size_t height_;
};