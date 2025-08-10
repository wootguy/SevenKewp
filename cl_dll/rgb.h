#pragma once
#include <stdint.h>

#undef RGB

struct RGB {
	uint8_t r, g, b;

	RGB() : r(0), g(0), b(0) {}
	RGB(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
	RGB(Vector v) : r(v.x), g(v.y), b(v.z) {}
	RGB(uint32_t hex) : r((hex >> 16) & 0xff), g((hex >> 8) & 0xff), b(hex & 0xff) {}

	inline Vector ToVector() { return Vector(r, g, b); }
};
