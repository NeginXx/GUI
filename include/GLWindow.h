#pragma once

struct SDL_Window;

class GLWindow {
 public:
	GLWindow(size_t width, size_t height);
	size_t GetWidth() const;
	size_t GetHeight() const;
	SDL_Window* GetWindow() const;
	void RenderPresent(Render* render);
	~GLWindow();

 private:
	SDL_Window* window_ = nullptr;
};
