#pragma once
class Render;
class SDL_Texture;

class Texture {
 public:
  Texture() = delete;
 	Texture(const char* image_name, Render* render);
  Texture(const char* text, Render* render, const Color& color);
  Texture(uint width, uint height, Render* render, const Color& color = {});
  Texture(const Texture& texture);
 	~Texture();
  void CopyTexture(const Texture& texture, const Rectangle* dst);
  void Draw(const Rectangle* src,
            const Rectangle* dst);
  void DrawWithNoScale(const Rectangle* dst);
  void DrawLine(const Point2D<int>& coordinates1,
                const Point2D<int>& coordinates2,
                const Color& color = {});
  void DrawThickLine(const Point2D<int>& coordinates1,
                     const Point2D<int>& coordinates2,
                     uint thickness,
                     const Color& color = {});
  void DrawCircle(Point2D<int> center, uint radius,
                  const Color& color = {});
  void DrawPoint(const Point2D<int>& coordinate,
                 const Color& color = {});
  void DrawText(const char* text_str,
                const Point2D<int>& dest_coord,
                const Color& color);
  void SetBackgroundColor(const Color& color);
  void SaveToPNG(const char* file_name);
  uint GetWidth() const;
  uint GetHeight() const;
 	friend class Render;

 private:
 	SDL_Texture* texture_;
  Render* render_;
  uint width_;
  uint height_;
};