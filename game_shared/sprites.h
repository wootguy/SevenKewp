#pragma once
#include <stdint.h>

enum SpriteFormat
{
	SPR_FMT_NORMAL,
	SPR_FMT_ADDITIVE,
	SPR_FMT_INDEXALPHA,
	SPR_FMT_ALPHATEST
};

enum SpriteMode
{
	SPR_MODE_PARALLEL_UPRIGHT,
	SPR_MODE_FACING_UPRIGHT,
	SPR_MODE_PARALLEL,
	SPR_MODE_ORIENTED,
	SPR_MODE_PARALLEL_ORIENTED,
	SPR_MODE_AUTO,				// use whatever is stored in the sprite
};

// format for reading/writing sprite files
// The engine stores a msprite_t instead.

#pragma pack(push, 1)
struct SpriteHeader {
	char ident[4];		// should always be "IDSP"
	uint32_t version;	// should be 2
	uint32_t mode;		// see sprite_modes enum
	uint32_t format;	// see sprite_formats enum
	float radius;		// ???
	uint32_t width;
	uint32_t height;
	uint32_t frames;
	float beamLength;	// for beam effects?
	uint32_t syncType;	// 0 = synchronized, 1 = random
	uint16_t paletteSz;	// always 256??
};

struct FrameHeader {
	uint32_t group;
	int32_t x;			// Sprite offset
	int32_t y;
	uint32_t width;
	uint32_t height;
};
#pragma pack(pop)