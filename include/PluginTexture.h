#pragma once
#include "main.h"
#include "Plugin.h"
#include "Texture.h"

class PluginTexture : public Plugin::ITexture {
 public:
 	PluginTexture(uint width, uint height, Render* render,
 		            const Color& color)
 	: texture_(width, height, render, color) {}

  int GetWidth() override {
  	return texture_.GetWidth();
  }

  int GetHeight() override {
  	return texture_.GetHeight();
  }

  Plugin::Buffer ReadBuffer() override {return {};}
  void ReleaseBuffer(Plugin::Buffer buffer) override {}
  void LoadBuffer(Plugin::Buffer buffer) override {}

  void Clear(Plugin::Color color) override {
  	texture_.SetBackgroundColor(GetColor(color));
  }

  void Present() override {}

  void DrawLine(int x0, int y0, int x1, int y1, Plugin::Color color) override {
  	texture_.DrawLine(Point2D<int>{x0, y0}, Point2D<int>{x1, y1}, GetColor(color));
  }

  void DrawThickLine(int x0, int y0, int x1, int y1, int thickness, Plugin::Color color) override {
  	assert(thickness >= 0);
  	texture_.DrawThickLine(Point2D<int>{x0, y0}, Point2D<int>{x1, y1}, (uint)thickness, GetColor(color));
  }

  void DrawCircle(int x, int y, int radius, Plugin::Color color) override {
  	assert(radius >= 0);
  	texture_.DrawCircle(Point2D<int>{x, y}, (uint)radius, GetColor(color));
  }

  void DrawRect(int x, int y, int width, int height, Plugin::Color color) override {
  	assert(width >= 0);
  	assert(height >= 0);
  	texture_.DrawRect({{x, y}, (uint)width, (uint)height}, GetColor(color));
  }

  void CopyTexture(ITexture* texture, int x, int y, int width, int height) override {
  	assert(!"CopyTexture function is yet to be written, sorry");
  }

  void Draw(const Rectangle& position) {
  	texture_.DrawWithNoScale(&position);
  }

 private:
 	Texture texture_;
};