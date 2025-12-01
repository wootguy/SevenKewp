#pragma once
#include "Platform.h"
#include "vector.h"
#include <stdint.h>

#undef RGB

struct EXPORT RGB {
	uint8_t r, g, b;

	RGB() : r(0), g(0), b(0) {}
	RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
	RGB(Vector v) : r(v.x), g(v.y), b(v.z) {}
	RGB(uint32_t hex) : r((hex >> 16) & 0xff), g((hex >> 8) & 0xff), b(hex & 0xff) {}

	inline Vector ToVector() { return Vector(r, g, b); }
};

struct EXPORT RGBA {
	uint8_t r, g, b, a;

	RGBA() : r(0), g(0), b(0), a(0) {}
	RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
	RGBA(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), a(255) {}
	RGBA(Vector v) : r(v.x), g(v.y), b(v.z), a(255) {}
	RGBA(Vector v, uint8_t a) : r(v.x), g(v.y), b(v.z), a(a) {}
	RGBA(RGB rgb) : r(rgb.r), g(rgb.g), b(rgb.b), a(255) {}
	RGBA(RGB rgb, uint8_t a) : r(rgb.r), g(rgb.g), b(rgb.b), a(a) {}
	RGBA(uint8_t rgfl[4]) { r = rgfl[0]; g = rgfl[1]; b = rgfl[2]; a = rgfl[3]; }

	inline Vector ToVector() { return Vector(r, g, b); }
};