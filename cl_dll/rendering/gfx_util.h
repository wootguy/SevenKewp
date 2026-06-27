#pragma once
#include "rgb.h"
#include "vector.h"

void gfx_draw_line(const Vector& a, const Vector& b, const RGBA& color, float width);
void gfx_draw_cube(const Vector& mins, const Vector& maxs, const RGBA& color);
void gfx_draw_cube_outline(const Vector& mins, const Vector& maxs, const RGBA& color, float thickness=1.0f);

// draw a 3D quad in front of the camera as if it was a HUD sprite
// Allows scaling HUD sprites in software mode
void gfx_draw_fake_hud_quad(float minX, float minY, float maxX, float maxY);

// draw view model sprite
void gfx_draw_sprite_weapon();

// convert a screen space coordinate to world space
Vector ScreenToWorld(const Vector& screen);

// convert a world space point to screen space for the local player
Vector WorldToScreen(const Vector& P);

// Get lightmap color underneath entity
RGB GetEntityLighting(cl_entity_t* ent);