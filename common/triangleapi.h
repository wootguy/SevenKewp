/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#if !defined( TRIANGLEAPIH )
#define TRIANGLEAPIH
#ifdef _WIN32
#pragma once
#endif

typedef enum 
{
	TRI_FRONT = 0,
	TRI_NONE = 1,
} TRICULLSTYLE;

#define TRI_API_VERSION		1

#define TRI_TRIANGLES		0
#define TRI_TRIANGLE_FAN	1
#define TRI_QUADS			2
#define TRI_POLYGON			3
#define TRI_LINES			4	
#define TRI_TRIANGLE_STRIP	5
#define TRI_QUAD_STRIP		6

typedef struct triangleapi_s
{
	/**
	*	Interface version.
	*	Should always be TRI_API_VERSION.
	*	@see TRI_API_VERSION
	*/
	int			version;

	/**
	*	Sets the render mode.
	*	@param mode Render mode to set.
	*	@see RenderMode
	*/
	void		( *RenderMode )( int mode );

	/**
	*	Begins a render operation.
	*	@param primitiveCode The type of operation to begin.
	*	@see TriangleMode
	*/
	void		( *Begin )( int primitiveCode );

	/**
	*	Ends a render operation.
	*/
	void		( *End ) ( void );

	/**
	*	Sets the RGBA color.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*	@param a Alpha color. [ 0, 1 ].
	*/
	void		( *Color4f ) ( float r, float g, float b, float a );

	/**
	*	Sets the RGBA color.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param a Alpha color. [ 0, 255 ].
	*/
	void		( *Color4ub ) ( unsigned char r, unsigned char g, unsigned char b, unsigned char a );

	/**
	*	Sets the texture coordinate to use for the next vertex.
	*	@param u U coordinate. [ 0, 1 ].
	*	@param v V coordinate. [ 0, 1 ].
	*/
	void		( *TexCoord2f ) ( float u, float v );

	/**
	*	Draws a vertex.
	*	@param vecWorldPoint Point in the world.
	*/
	void		( *Vertex3fv ) ( float *worldPnt );

	/**
	*	Draws a vertex.
	*	@param x X coordinate in the world.
	*	@param y Y coordinate in the world.
	*	@param z Z coordinate in the world.
	*/
	void		( *Vertex3f ) ( float x, float y, float z );

	/**
	*	Sets the global brightness value.
	*/
	void		( *Brightness ) ( float brightness );

	/**
	*	Sets the cull face.
	*	@param style Cull style.
	*	@see TRICULLSTYLE
	*/
	void		( *CullFace ) ( TRICULLSTYLE style );

	/**
	*	Sets the given sprite frame as the active texture.
	*	@param pSpriteModel Sprite to use.
	*	@param frame Sprite frame to use.
	*	@return true on success, false otherwise.
	*/
	int			( *SpriteTexture ) ( struct model_s *pSpriteModel, int frame );

	/**
	*	Converts a world coordinate to a screen coordinate.
	*	@param vecWorld World coordinate.
	*	@param[ out ] vecScreen Screen coordinate.
	*	@return true if it's Z clipped, false otherwise.
	*/
	int			( *WorldToScreen ) ( float *world, float *screen );  // Returns 1 if it's z clipped

	/**
	*	Sets the fog color.
	*	Works just like GL_FOG, vecFogColor is r/g/b.
	*	@param vecFogColor Fog color. [ 0, 255 ].
	*	@param flStart Distance from the player where the fog should start.
	*	@param flEnd Distance from the player where the fog should be fully opaque.
	*	@param bOn Whether to enable or disable the fog.
	*/
	void		( *Fog ) ( float flFogColor[3], float flStart, float flEnd, int bOn ); // Works just like GL_FOG, flFogColor is r/g/b.
	
	/**
	*	Converts a screen coordinate to a world coordinate.
	*	@param vecScreen Screen coordinate.
	*	@param[ out ] vecWorld World coordinate.
	*/
	void		( *ScreenToWorld ) ( float *screen, float *world  ); 

	/**
	*	Gets the value of a matrix.
	*	@param pname Constant identifying the matrix. Must be an OpenGL constant.
	*	@param matrix Pointer to the 4x4 matrix.
	*	TODO use GLM matrix. - Solokiller
	*/
	void		( *GetMatrix ) ( const int pname, float *matrix );

	/**
	*	Gets whether the given box is inside the PVS.
	*	@param vecMins Minimum bounds.
	*	@param vecMaxs Maximum bounds.
	*	@return true if the box is inside the PVS, false otherwise.
	*/
	int			( *BoxInPVS ) ( float *mins, float *maxs );

	/**
	*	gets the light value at the given position.
	*	@param vecPos Position.
	*	@param[ out ] vecValue RGB light value. [ 0, 255 ].
	*/
	void		( *LightAtPoint ) ( float *pos, float *value );

	/**
	*	Sets the RGBA color, taking the render mode as a hint for the interpretation of the values.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*	@param a Alpha color. [ 0, 1 ].
	*	@param mode Render mode to use as a hint.
	*	@see RenderMode
	*/
	void		( *Color4fRendermode ) ( float r, float g, float b, float a, int rendermode );

	/**
	*	Used with Fog()...sets fog density and whether the fog should be applied to the skybox.
	*	@param flDensity Fog density.
	*	@param bFogSkybox Whether the fog should be applied to the skybox.
	*/
	void		( *FogParams ) ( float flDensity, int iFogSkybox ); // Used with Fog()...sets fog density and whether the fog should be applied to the skybox

} triangleapi_t;

#endif // !TRIANGLEAPIH
