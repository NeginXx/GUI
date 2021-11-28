#pragma once
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstdint>
#include <cstring>
#include "StackTrace.h"

const float kPi = 3.1416f;

struct Color {
	unsigned char red = 0;
	unsigned char green = 0;					
	unsigned char blue = 0;
	unsigned char alpha = 255;
};

Color GetColor(uint color);
uint GetColor(Color color);

const Color kLightPurple = {171, 60, 255};
const Color kLightGreen  = {107, 216, 79};
const Color kLightPink   = {255, 153, 204};
const Color kBlue        = {0, 0, 255};
const Color kRed         = {255, 0, 0};
const Color kBlack       = {0, 0, 0};
const Color kWhite       = {255, 255, 255};

template<typename T>
inline T Min(const T& lhs, const T& rhs) {
	return lhs < rhs ? lhs : rhs;
}

template<typename T>
inline T Max(const T& lhs, const T& rhs) {
	return lhs < rhs ? rhs : lhs;
}

template<typename T>
inline bool IsInBound(const T& min, const T& mid, const T& max) {
	assert(min < max);
	return min <= mid && mid <= max;
}

template<typename T>
inline void Stabilize(const T& min, T* mid, const T& max) {
	assert(min < max);
	if (*mid < min) {
		*mid = min;
	} else if (*mid > max) {
		*mid = max;
	}
}

template<typename T>
struct Vec2D;

template <typename T>
struct Point2D {
	T x;
	T y;
	Point2D() = default;
	Point2D(T x_, T y_) {
		x = x_;
		y = y_;
	}
	Point2D(const Vec2D<T>& v) {
		x = v.x;
		y = v.y;
	}
	void operator=(const Vec2D<T>& v) {
		x = v.x;
		y = v.y;
	}
	Point2D<T>& operator+=(const Point2D<T>& p) {
		x += p.x;
		y += p.y;
		return *this;
	}
	Point2D<T>& operator-=(const Point2D<T>& p) {
		x -= p.x;
		y -= p.y;
		return *this;
	}

	template <typename T1>
	explicit operator Point2D<T1>() const {
	  return {static_cast<T1>(x), static_cast<T1>(y)};
	}
};

template <typename T>
struct Vec2D {
	T x;
	T y;
	Vec2D() = default;
	Vec2D(T x_, T y_) {
		x = x_;
		y = y_;
	}
	Vec2D(Point2D<T> a) {
		x = a.x;
		y = a.y;
	}
	void operator=(const Point2D<T>& p) {
		x = p.x;
		y = p.y;
	}
	T GetLength() {
		return sqrt(x * x + y * y);
	}
	void Normalize() {
		T len = GetLength();
		x /= len;
		y /= len;
	}
};

struct Rectangle {
  Point2D<int> corner;
  uint width;
  uint height;
};

template <typename T>
Point2D<T> operator+(const Point2D<T>& lhs,
	                   const Point2D<T>& rhs) {
	Point2D<T> res = lhs;
	return res += rhs;
}

template <typename T>
Point2D<T> operator-(const Point2D<T>& lhs,
	                   const Point2D<T>& rhs) {
	Point2D<T> res = lhs;
	return res -= rhs;
}

template <typename T>
Point2D<T> operator/(const Point2D<T>& lhs,
	                   T val) {
	return Point2D<T>{lhs.x / val, lhs.y / val};
}

template <typename T>
bool operator<(const Point2D<T>& lhs,
	             const Point2D<T>& rhs) {
	return lhs.x < rhs.x && lhs.y < rhs.y;
}

template <typename T>
bool operator<=(const Point2D<T>& lhs,
	              const Point2D<T>& rhs) {
	return lhs.x <= rhs.x && lhs.y <= rhs.y;
}

template <typename T>
bool operator>(const Point2D<T>& lhs,
	             const Point2D<T>& rhs) {
	return lhs.x > rhs.x && lhs.y > rhs.y;
}

template <typename T>
bool operator>=(const Point2D<T>& lhs,
	              const Point2D<T>& rhs) {
	return lhs.x >= rhs.x && lhs.y >= rhs.y;
}

template <typename T>
bool operator==(const Point2D<T>& lhs,
	              const Point2D<T>& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <typename T>
bool operator!=(const Point2D<T>& lhs,
	              const Point2D<T>& rhs) {
	return lhs.x != rhs.x && lhs.y != rhs.y;
}