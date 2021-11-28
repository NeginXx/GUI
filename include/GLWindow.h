#pragma once

struct SDL_Window;

class GLWindow {
 public:
	GLWindow(uint width, uint height);
	uint GetWidth() const;
	uint GetHeight() const;
	SDL_Window* GetWindow() const;
	void RenderPresent(Render* render);
	~GLWindow();

 private:
	SDL_Window* window_ = nullptr;
};
