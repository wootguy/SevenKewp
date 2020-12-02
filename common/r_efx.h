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
#if !defined ( R_EFXH )
#define R_EFXH
#ifdef _WIN32
#pragma once
#endif

// particle_t
#if !defined( PARTICLEDEFH )  
#include "particledef.h"
#endif

// BEAM
#if !defined( BEAMDEFH )
#include "beamdef.h"
#endif

// dlight_t
#if !defined ( DLIGHTH )
#include "dlight.h"
#endif

// cl_entity_t
#if !defined( CL_ENTITYH )
#include "cl_entity.h"
#endif

/*
// FOR REFERENCE, These are the built-in tracer colors.  Note, color 4 is the one
//  that uses the tracerred/tracergreen/tracerblue and traceralpha cvar settings
color24 gTracerColors[] =
{
	{ 255, 255, 255 },		// White
	{ 255, 0, 0 },			// Red
	{ 0, 255, 0 },			// Green
	{ 0, 0, 255 },			// Blue
	{ 0, 0, 0 },			// Tracer default, filled in from cvars, etc.
	{ 255, 167, 17 },		// Yellow-orange sparks
	{ 255, 130, 90 },		// Yellowish streaks (garg)
	{ 55, 60, 144 },		// Blue egon streak
	{ 255, 130, 90 },		// More Yellowish streaks (garg)
	{ 255, 140, 90 },		// More Yellowish streaks (garg)
	{ 200, 130, 90 },		// More red streaks (garg)
	{ 255, 120, 70 },		// Darker red streaks (garg)
};
*/

// Temporary entity array
#define TENTPRIORITY_LOW	0
#define TENTPRIORITY_HIGH	1

// TEMPENTITY flags
#define	FTENT_NONE				0x00000000
#define	FTENT_SINEWAVE			0x00000001
#define	FTENT_GRAVITY			0x00000002
#define FTENT_ROTATE			0x00000004
#define	FTENT_SLOWGRAVITY		0x00000008
#define FTENT_SMOKETRAIL		0x00000010
#define FTENT_COLLIDEWORLD		0x00000020
#define FTENT_FLICKER			0x00000040
#define FTENT_FADEOUT			0x00000080
#define FTENT_SPRANIMATE		0x00000100
#define FTENT_HITSOUND			0x00000200
#define FTENT_SPIRAL			0x00000400
#define FTENT_SPRCYCLE			0x00000800
#define FTENT_COLLIDEALL		0x00001000 // will collide with world and slideboxes
#define FTENT_PERSIST			0x00002000 // tent is not removed when unable to draw 
#define FTENT_COLLIDEKILL		0x00004000 // tent is removed upon collision with anything
#define FTENT_PLYRATTACHMENT	0x00008000 // tent is attached to a player (owner)
#define FTENT_SPRANIMATELOOP	0x00010000 // animating sprite doesn't die when last frame is displayed
#define FTENT_SPARKSHOWER		0x00020000
#define FTENT_NOMODEL			0x00040000 // Doesn't have a model, never try to draw ( it just triggers other things )
#define FTENT_CLIENTCUSTOM		0x00080000 // Must specify callback.  Callback function is responsible for killing tempent and updating fields ( unless other flags specify how to do things )

typedef struct tempent_s
{
	int			flags;
	float		die;
	float		frameMax;
	float		x;
	float		y;
	float		z;
	float		fadeSpeed;
	float		bounceFactor;
	int			hitSound;
	void		( *hitcallback )	( struct tempent_s *ent, struct pmtrace_s *ptr );
	void		( *callback )		( struct tempent_s *ent, float frametime, float currenttime );
	struct tempent_s	*next;
	int			priority;
	short		clientIndex;	// if attached, this is the index of the client to stick to
								// if COLLIDEALL, this is the index of the client to ignore
								// TENTS with FTENT_PLYRATTACHMENT MUST set the clientindex! 

	vec3_t		tentOffset;		// if attached, client origin + tentOffset = tent origin.
	cl_entity_t	entity;

	// baseline.origin		- velocity
	// baseline.renderamt	- starting fadeout intensity
	// baseline.angles		- angle velocity
} TEMPENTITY;

typedef struct efx_api_s efx_api_t;

struct efx_api_s
{
	/**
	*	Creates a new custom particle.
	*	@param callback Callback to set.
	*	@return Particle.
	*/
	particle_t  *( *R_AllocParticle )			(void (*callback) (struct particle_s* particle, float frametime));

	/**
	*	Creates an explosion of blob particles.
	*	@param vecOrigin Explosion origin.
	*/
	void		( *R_BlobExplosion )			( const float* vecOrigin );

	/**
	*	Creates blood particles.
	*	@param vecOrigin Origin.
	*	@param vecDir Direction.
	*	@param pcolor Particle color.
	*	@param speed Movement speed.
	*/
	void		( *R_Blood )					( const float* vecOrigin, const float* vecDir, int pcolor, int speed );

	/**
	*	Creates blood sprites.
	*	@param vecOrigin Origin.
	*	@param colorindex Color index in the base palette.
	*	@param modelIndex First sprite index.
	*	@param modelIndex2 Second sprite index.
	*	@param size Size of the sprites.
	*/
	void		( *R_BloodSprite )				( const float* vecOrigin, int colorindex, int modelIndex, int modelIndex2, float size );

	/**
	*	Creates a blood stream.
	*	@param vecOrigin Origin.
	*	@param vecDir Direction.
	*	@param pcolor Particle color.
	*	@param speed Movement speed.
	*/
	void		( *R_BloodStream )				( const float* vecOrigin, const float* vecDir, int pcolor, int speed );

	/**
	*	Creates a break model.
	*	@param vecOrigin Origin.
	*	@param vecSize Size of the box in which to spawn models.
	*	@param vecDir Direction.
	*	@param random Random spread.
	*	@param life How long the models should stick around.
	*	@param count Number of models to spawn.
	*	@param modelIndex Index of the model to spawn.
	*	@param flags Flags. TODO find constants - Solokliler
	*/
	void		( *R_BreakModel )				( const float* vecOrigin, const float* vecSize, const float* vecDir, float random, float life, int count, int modelIndex, char flags );

	/**
	*	Creates bubbles within a box.
	*	@param vecMins Minimum bounds.
	*	@param vecMaxs Maximum bounds.
	*	@param height How high the bubbles should float.
	*	@param modelIndex Index of the bubbles model to use.
	*	@param count Number of bubbles to spawn.
	*	@param speed Speed of the bubbles.
	*/
	void		( *R_Bubbles )					( const float* vecMins, const float* vecMaxs, float height, int modelIndex, int count, float speed );

	/**
	*	Creates a trail of bubbles between 2 points.
	*	@param vecStart Starting position.
	*	@param vecEnd End position.
	*	@param height How high the bubbles should float.
	*	@param modelIndex Index of the bubbles model to use.
	*	@param count Number of bubbles to spawn.
	*	@param speed Speed of the bubbles.
	*/
	void		( *R_BubbleTrail )				( const float* vecStart, const float* vecEnd, float height, int modelIndex, int count, float speed );

	/**
	*	Creates bullet impact particles.
	*	@param vecOrigin Origin.
	*/
	void		( *R_BulletImpactParticles )	( const float* vecOrigin );

	/**
	*	Creates particles around the given entity.
	*	@param ent Entity.
	*/
	void		( *R_EntityParticles )			( struct cl_entity_s* ent );

	/**
	*	Creates an explosion effect.
	*	@param vecOrigin Origin.
	*	@param model Index of the model to use.
	*	@param scale Scale of the explosion.
	*	@param framerate Frame rate.
	*	@param flags Flags. TODO figure out which flags apply - Solokiller
	*/
	void		( *R_Explosion )				( const float* vecOrigin, int model, float scale, float framerate, int flags );

	/**
	*	Does not appear to do anything.
	*	TODO: TE_FIZZ uses this. Check that as well. - Solokiller
	*/
	void		( *R_FizzEffect )				(struct cl_entity_s *pent, int modelIndex, int density );
	
	/**
	*	Creates a field of fire.
	*	@param vecOrigin Origin.
	*	@param radius Fire is made in a square around the origin. -radius, -radius to radius, radius.
	*	@param modelIndex Index of the model to spawn.
	*	@param count Number of models to spawn.
	*	@param flags Flags. @see TEFireFlag
	*	@param life How long the models should life for, in seconds.
	*/
	void		( *R_FireField ) 				( const float* vecOrigin, int radius, int modelIndex, int count, int flags, float life );

	/**
	*	Creates particles that flicker upwards, confetti style.
	*	@param vecOrigin Origin to spawn the particles at.
	*/
	void		( *R_FlickerParticles )			( const float* vecOrigin );

	/**
	*	Creates a funnel effect. Sprites moving along the surface of a cone that points downwards.
	*	@param vecOrigin Origin that the sprites should move towards or away from.
	*	@param modelIndex Index of the model to use.
	*	@param bReverse If false, the sprites move towards the origin. If true, the sprites move away from the origin.
	*/
	void		( *R_FunnelSprite )				( const float* vecOrigin, int modelIndex, int bReverse );

	/**
	*	Creates an implosion effect. Tracers moving towards a point.
	*	@param vecOrigin Point to move to.
	*	@param radius Radius around the point to spawn the tracers at.
	*	@param count Number of tracers.
	*	@param life How long the tracers should live for, in seconds.
	*/
	void		( *R_Implosion )				( const float* vecOrigin, float radius, int count, float life );

	/**
	*	Creates a large funnel of green particles.
	*	@param vecOrigin Origin.
	*	@param bReverse If false, particles move towards the origin. If true, particles move away from the origin.
	*	@see R_FunnelSprite
	*/
	void		( *R_LargeFunnel )				( const float* vecOrigin, int bReverse );

	/**
	*	Quake 1 lava splash. Creates red particles moving slightly upward.
	*	@param vecOrigin Origin to spawn the particles around.
	*/
	void		( *R_LavaSplash )				( const float* vecOrigin );

	/**
	*	Creates effects for multiple gunshots. Also plays ricochet sounds.
	*	@param vecOrigin Shoot origin.
	*	@param vecDir Direction to shoot in.
	*	@param vecNoise Noise.
	*	@param count Number of gunshots.
	*	@param decalCount Number of decals to use.
	*	@param pDecalIndices Array of decal indices to use.
	*/
	void		( *R_MultiGunshot )				( const float* vecOrigin, const float* vecDir, const float* vecNoise, int count, int decalCount, const int* pDecalIndices );

	/**
	*	Creates a muzzleflash sprite.
	*	@param vecOrigin Origin.
	*	@param type The scale and sprite to use.
	*				If not 0, calculates the scale as follows: ( type / 10 ) * 0.1
	*				Sprite is determined as follows: type % 10 % 3
	*				Maps to the following sprites:
	*				sprites/muzzleflash1.spr
	*				sprites/muzzleflash2.spr
	*				sprites/muzzleflash3.spr
	*/
	void		( *R_MuzzleFlash )				( const float* vecOrigin, int type );

	/**
	*	Creates particles in a box.
	*	@param vecMins Minimum bounds.
	*	@param vecMaxs Maximum bounds.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param life How long the particles should life for, in seconds.
	*/
	void		( *R_ParticleBox )				( const float* vecMins, const float* vecMaxs, unsigned char r, unsigned char g, unsigned char b, float life );

	/**
	*	Creates a burst of particles that move outward from the origin.
	*	Size has influence on speed. Will easily use up all available particles.
	*	@param vecOrigin Origin.
	*	@param size Size of the box that the particles should expand to.
	*	@param color Particle color.
	*	@param life How long the particles should live for, in seconds.
	*/
	void		( *R_ParticleBurst )			( const float* vecOrigin, int size, int color, float life );

	/**
	*	Creates yellowish particles around the origin that move upward.
	*	@param vecOrigin Origin.
	*/
	void		( *R_ParticleExplosion )		( const float* vecOrigin );

	/**
	*	Creates particles using the given color set around the origin that move upward.
	*	@param vecOrigin Origin.
	*	@param colorStart Index of the first color to use.
	*	@param colorLength Number of colors to use.
	*/
	void		( *R_ParticleExplosion2 )		( const float* vecOrigin, int colorStart, int colorLength );

	/**
	*	Creates a line of particles.
	*	@param vecStart Starting position.
	*	@param vecEnd End position.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param life How long the particles should live for, in seconds.
	*/
	void		( *R_ParticleLine )				( const float* vecStart, const float* vecEnd, unsigned char r, unsigned char g, unsigned char b, float life );

	/**
	*	Emits sprites from a player's bounding box (ONLY use for players!).
	*	@param client Client index. 1 based.
	*	@param modelIndex Index of the model to emit.
	*	@param count Number of sprites to emit.
	*	@param size Size of the sprites.
	*/
	void		( *R_PlayerSprites )			( int client, int modelIndex, int count, int size );

	/**
	*	Creates a projectile.
	*	@param vecOrigin Starting origin.
	*	@param vecVelocity Velocity.
	*	@param modelIndex Index of the model to use.
	*	@param life How long the projectile should live for, in seconds.
	*	@param owner If not 0, which entity should never be collided with. Must be a player.
	*	@param hitcallback Callback to invoke when it hits something.
	*	TODO use callback definition - Solokiller
	*/
	void		( *R_Projectile )				( const float* vecOrigin, const float* vecVelocity, int modelIndex, int life, int owner, void (*hitcallback)( TEMPENTITY *ent, struct pmtrace_s *ptr ) );
	
	/**
	*	Plays a ricochet sound at the given location.
	*	@param vecOrigin Origin.
	*/
	void		( *R_RicochetSound )			( const float* vecOrigin );

	/**
	*	Creates a ricochet sprite.
	*	@param vecOrigin Origin.
	*	@param pmodel Model to use.
	*	@param duration How long the sprite should life for, in seconds.
	*	@param scale Sprite scale.
	*/
	void		( *R_RicochetSprite )			( const float* vecOrigin, struct model_s* pmodel, float duration, float scale );

	/**
	*	Creates a rocket flare sprite at the given location. Will exist for 0.01 seconds.
	*	Uses the sprites/animglow01.spr sprite.
	*	@param vecOrigin Origin.
	*/
	void		( *R_RocketFlare )				( const float* vecOrigin );

	/**
	*	Creates a rocket trail.
	*	@param[ in, out ] vecStart Starting position. Modified to contain start + velocity.
	*	@param vecEnd End position.
	*	@param type Trail type.
	*	@see RocketTrailType
	*/
	void		( *R_RocketTrail )				( float* vecStart, const float* vecEnd, const int type );

	/**
	*	Particle effect that shows the appearance of running fast. See Team Fortress 2's Scout.
	*	@param vecOrigin Origin.
	*	@param vecDir Direction.
	*	@param color Particle color.
	*	@param count Number of particles.
	*/
	void		( *R_RunParticleEffect )		( const float* vecOrigin, const float* vecDir, int color, int count );

	/**
	*	Creates a line made up out of red particles.
	*	Lasts for 30 seconds.
	*	@param vecStart Starting position.
	*	@param vecEnd End position.
	*/
	void		( *R_ShowLine )					( const float* vecStart, const float* vecEnd );

	/**
	*	Creates a spark effect. Combines spark streaks and a ricochet sprite.
	*	@param vecOrigin Origin.
	*	@param count Number of sparks.
	*	@param velocityMin Minimum velocity.
	*	@param velocityMin Maximum velocity.
	*/
	void		( *R_SparkEffect )				( const float* vecOrigin, int count, int velocityMin, int velocityMax );

	/**
	*	Creates a shower of sparks.
	*	@param vecOrigin Origin.
	*/
	void		( *R_SparkShower )				( const float* vecOrigin );

	/**
	*	Creates spark streaks.
	*	@param vecOrigin Origin.
	*	@param count Number of sparks.
	*	@param velocityMin Minimum velocity.
	*	@param velocityMin Maximum velocity.
	*/
	void		( *R_SparkStreaks )				( const float* vecOrigin, int count, int velocityMin, int velocityMax );

	/**
	*	Sprays models out like a gib shooter.
	*	@param vecOrigin Origin.
	*	@param vecDir Direction.
	*	@param modelIndex Index of the model to spray.
	*	@param count Number of models to spray.
	*	@param speed Spray speed.
	*	@param spread Random spread.
	*	@param rendermode Render mode. @see RenderMode
	*/
	void		( *R_Spray )					( const float* vecOrigin, const float* vecDir, int modelIndex, int count, int speed, int spread, int rendermode );

	/**
	*	Sets the temp entity's parameters for an explosion sprite.
	*	@param pTemp Temporary entity.
	*	@param scale Scale.
	*	@param flags Flags. @see SpriteExplodeFlag
	*/
	void		( *R_Sprite_Explode )			( TEMPENTITY* pTemp, float scale, int flags );

	/**
	*	Sets the temp entity's parameters for a smoke sprite.
	*	Sets a dark gray color.
	*	@param pTemp Temporary entity.
	*	@param scale Scale.
	*/
	void		( *R_Sprite_Smoke )				( TEMPENTITY* pTemp, float scale );

	/**
	*	Variant of R_Spray that handles sprite properties automatically.
	*	Sets alpha test render mode.
	*	@param vecOrigin Origin.
	*	@param vecDir Direction.
	*	@param modelIndex Index of the model to spray.
	*	@param count Number of models to spray.
	*	@param speed Spray speed.
	*	@param iRand Random spread.
	*/
	void		( *R_Sprite_Spray )				( const float* vecOrigin, const float* vecDir, int modelIndex, int count, int speed, int iRand );

	/**
	*	Creates a sinusoidal wave of models between 2 points. The wave will fade out gradually from the start to end position.
	*	@param type Never used.
	*	@param vecStart Starting position.
	*	@param vecEnd End position.
	*	@param modelIndex Index of the model.
	*	@param count Number of models.
	*	@param life How long the models should live for, in seconds.
	*	@param size Scale.
	*	@param amplitude Wave amplitude.
	*	@param renderamt Render amount.
	*	@param speed Speed.
	*/
	void		( *R_Sprite_Trail )				( int type, const float* vecStart, const float* vecEnd, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed );

	/**
	*	Sets the temp entity's parameters for a wall puff effect.
	*	Lasts for 0.01 seconds.
	*	@param pTemp Temporary entity.
	*	@param scale Scale.
	*/
	void		( *R_Sprite_WallPuff )			( TEMPENTITY* pTemp, float scale );

	/**
	*	Variant of R_SparkStreaks with more options.
	*	@param vecOrigin Origin.
	*	@param vecDir Direction.
	*	@param color Color.
	*	@param count Number of streaks.
	*	@param speed Streak speed.
	*	@param velocityMin Minimum velocity.
	*	@param velocityMax Maximum velocity.
	*/
	void		( *R_StreakSplash )				( const float* vecOrigin, const float* vecDir, int color, int count, float speed, int velocityMin, int velocityMax );

	/**
	*	Creates a tracer effect between the given positions. Moves from start to end.
	*	@param[ in, out ] Starting position. Will contain start + velocity.
	*	@param vecEnd End position.
	*/
	void		( *R_TracerEffect )				( float* vecStart, const float* vecEnd );

	/**
	*	@param vecOrigin Origin.
	*	@param vecVelocity Velocity.
	*	@param life How long the particle should live for, in seconds.
	*	@param colorIndex Particle color.
	*	@param length Length of the particle.
	*	@param deathcontext user defined context value.
	*	@param deathfunc Callback to invoke when the particle dies.
	*/
	void		( *R_UserTracerParticle )		( const float* vecOrigin, const float* vecVelocity, float life, int colorIndex, float length,
												  unsigned char deathcontext, void ( *deathfunc)( particle_t *particle ) );

	/**
	*	Creates tracer particles.
	*	@param vecOrigin Origin.
	*	@param vecVelocity Velocity.
	*	@param life How long the particle should live for, in seconds.
	*	@return Particle.
	*/
	particle_t *( *R_TracerParticles )			( const float* vecOrigin, const float* vecVelocity, float life );

	/**
	*	Creates a Quake 1 teleport splash effect.
	*/
	void		( *R_TeleportSplash )			( const float* vecOrigin );

	/**
	*	Creates particles and count temp entities with the given model that fall down.
	*	@param vecOrigin Origin.
	*	@param speed Speed.
	*	@param life How long the effect should live for, in seconds.
	*	@param count Number of temporary entities to create.
	*	@param modelIndex Index of the model to use.
	*/
	void		( *R_TempSphereModel )			( const float* vecOrigin, float speed, float life, int count, int modelIndex );
	
	/**
	*	Creates a temporary entity with the given model.
	*	@param vecOrigin Origin.
	*	@param vecDir Direction.
	*	@param vecAngles Angles.
	*	@param life How long the entity should live for, in seconds.
	*	@param modelIndex Index of the model to use.
	*	@param soundtype Bounce sound type. @see TE_Bounce
	*	@return Temporary entity. Can be null.
	*/
	TEMPENTITY*	( *R_TempModel )				( const float* vecOrigin, const float* vecDir, const float* vecAngles, float life, int modelIndex, int soundtype );

	/**
	*	Creates a temp entity with a sprite model with default settings.
	*	@param vecOrigin Origin.
	*	@param spriteIndex Index of the sprite to use.
	*	@param framerate Frame rate.
	*	@return Temporary entity. Can be null.
	*/
	TEMPENTITY*	( *R_DefaultSprite )			( const float* vecOrigin, int spriteIndex, float framerate );

	/**
	*	Creates a temp entity with a sprite model with given settings.
	*	@param vecOrigin Origin.
	*	@param vecDir Direction.
	*	@param scale Scale.
	*	@param modelIndex Index of the model to use.
	*	@param rendermode Render mode.
	*	@param renderfx Render FX.
	*	@param a Alpha value. [ 0, 1 ].
	*	@param life How long the entity should live for, in seconds.
	*	@param flags Flags. @see TempEntFlag.
	*	@return Temporary entity. Can be null.
	*/
	TEMPENTITY*	( *R_TempSprite )				( const float* vecOrigin, const float* vecDir, float scale, int modelIndex, int rendermode, int renderfx, float a, float life, int flags );

	/**
	*	Converts a decal index to a texture index.
	*	@param id Decal index.
	*	@return Texture index.
	*/
	int			( *Draw_DecalIndex )			( int id );

	/**
	*	Gets the decal index of a decal.
	*	@param pszName Decal name.
	*	@return Decal index, or 0 if the decal couldn't be found.
	*/
	int			( *Draw_DecalIndexFromName )	( const char* const pszName );

	/**
	*	Projects a decal onto the given brush entity's model.
	*	@param textureIndex Decal to project.
	*	@param entity Index of the entity to project onto.
	*	@param modelIndex Index of the entity's model to project onto.
	*	@param vecPosition Position in the world to project at.
	*	@param flags Flags. TODO: figure these out. - Solokiller
	*/
	void		( *R_DecalShoot )				( int textureIndex, int entity, int modelIndex, const float* vecPosition, int flags );

	/**
	*	Attaches a temp entity to a player.
	*	@param client Index of the client to attach to. 1 based.
	*	@param modelIndex Index of the model to attach.
	*	@param zoffset Z offset of the entity.
	*	@param life How long the entity should live for, in seconds.
	*/
	void		( *R_AttachTentToPlayer )		( int client, int modelIndex, float zoffset, float life );

	/**
	*	Removes all temp entities that have been attached to the given player.
	*	@param client Index of the client. 1 based.
	*/
	void		( *R_KillAttachedTents )		( int client );

	/**
	*	Creates a circular beam.
	*	@param type Beam type. @see BeamCircleType
	*	@param vecStart Starting point.
	*	@param vecEnd End point.
	*	@param modelIndex Index of the sprite to use.
	*	@param life How long the beam should live for, in seconds.
	*	@param width Beam width.
	*	@param amplitude Beam amplitude. If non-zero, creates sinusoidal wave effect.
	*	@param brightness Brightness.
	*	@param speed Beam speed.
	*	@param startFrame Starting frame.
	*	@param framerate Frame rate.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*	@return Beam, or null if no beam could be created.
	*/
	BEAM*		( *R_BeamCirclePoints )		( int type, float* vecStart, float* vecEnd, int modelIndex,
												  float life, float width, float amplitude, float brightness, float speed, 
												  int startFrame, float framerate, float r, float g, float b );

	/**
	*	Creates a beam between a fixed point and an entity.
	*	@param startEnt Entity to attach to.
	*	@param vecEnd Point to attach to.
	*	@param modelIndex Index of the sprite to use.
	*	@param life How long the beam should live for, in seconds.
	*	@param width Beam width.
	*	@param amplitude Beam amplitude. If non-zero, creates sinusoidal wave effect.
	*	@param brightness Brightness.
	*	@param speed Beam speed.
	*	@param startFrame Starting frame.
	*	@param framerate Frame rate.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*	@return Beam, or null if no beam could be created.
	*/
	BEAM*		( *R_BeamEntPoint )			( int startEnt, const float* vecEnd, int modelIndex,
												  float life, float width, float amplitude, float brightness, float speed, 
												  int startFrame, float framerate, float r, float g, float b );

	/**
	*	Creates a beam between 2 entities.
	*	@param startEnt Entity to attach to.
	*	@param endEnt Other entity to attach to.
	*	@param modelIndex Index of the sprite to use.
	*	@param life How long the beam should live for, in seconds.
	*	@param width Beam width.
	*	@param amplitude Beam amplitude. If non-zero, creates sinusoidal wave effect.
	*	@param brightness Brightness.
	*	@param speed Beam speed.
	*	@param startFrame Starting frame.
	*	@param framerate Frame rate.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*	@return Beam, or null if no beam could be created.
	*/
	BEAM*		( *R_BeamEnts )				( int startEnt, int endEnt, int modelIndex, 
												  float life, float width, float amplitude, float brightness, float speed, 
												  int startFrame, float framerate, float r, float g, float b );

	/**
	*	Creates a beam that follows the entity. Doesn't appear to actually create a beam.
	*	TODO
	*	@param startEnt Entity to attach to.
	*	@param modelIndex Index of the sprite to use.
	*	@param life How long the beam should live for, in seconds.
	*	@param width Beam width.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*	@return Beam, or null if no beam could be created.
	*/
	BEAM*		( *R_BeamFollow )				( int startEnt, int modelIndex, float life, float width, float r, float g, float b, float brightness );

	/**
	*	Removes all beams that were attached to the given entity.
	*	@param deadEntity Index of the entity.
	*/
	void		( *R_BeamKill )					( int deadEntity );

	/**
	*	Creates a lightning bolt.
	*	Does not appear to do anything.
	*	TODO
	*	@param vecStart Starting position.
	*	@param vecEnd End position.
	*	@param modelIndex Index of the sprite to use.
	*	@param life How long the beam should live for, in seconds.
	*	@param width Beam width.
	*	@param amplitude Sinusoidal amplitude.
	*	@param brightness Brightness.
	*	@param speed Speed.
	*	@return Beam, or null if no beam could be created.
	*/
	BEAM*		( *R_BeamLightning )			( const float* vecStart, const float* vecEnd, int modelIndex, float life, float width, float amplitude, float brightness, float speed );

	/**
	*	Creates a beam between 2 points.
	*	@param vecStart Starting position.
	*	@param vecEnd End position.
	*	@param modelIndex Index of the sprite to use.
	*	@param life How long the beam should live for, in seconds.
	*	@param width Beam width.
	*	@param amplitude Sinusoidal amplitude.
	*	@param brightness Brightness.
	*	@param speed Speed.
	*	@param startFrame Starting frame.
	*	@param framerate Frame rate.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*	@return Beam, or null if no beam could be created.
	*/
	BEAM*		( *R_BeamPoints )				( const float* vecStart, const float* vecEnd, int modelIndex,
												  float life, float width, float amplitude, float brightness, float speed, 
												  int startFrame, float framerate, float r, float g, float b );

	/**
	*	Creates a beam ring between the given entities.
	*	The starting and end entities define the diameter of the circle, with the circle being created between the points.
	*	@param startEnt Index of the starting entity.
	*	@param endEnt Index of the end entity.
	*	@param modelIndex Index of the sprite to use.
	*	@param life How long the beam should live for, in seconds.
	*	@param width Beam width.
	*	@param amplitude Sinusoidal amplitude.
	*	@param brightness Brightness.
	*	@param speed Speed.
	*	@param startFrame Starting frame.
	*	@param framerate Frame rate.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*	@return Beam, or null if no beam could be created.
	*/
	BEAM*		( *R_BeamRing )				( int startEnt, int endEnt, int modelIndex, 
											  float life, float width, float amplitude, float brightness, float speed, 
											  int startFrame, float framerate, float r, float g, float b );

	/**
	*	Creates a dynamic light.
	*	@param key if non-zero, looks up the light with the given key. If 0, or the light couldn't be found, gets the first free light, or the first light if no free lights exist.
	*	@return Dynamic light.
	*/
	dlight_t*	( *CL_AllocDlight )			( int key );

	/**
	*	Creates a dynamic entity light.
	*	@param key if non-zero, looks up the light with the given key. If 0, or the light couldn't be found, gets the first free light, or the first light if no free lights exist.
	*	@return Dynamic entity light.
	*/
	dlight_t*	( *CL_AllocElight )			( int key );

	/**
	*	Allocates a low priority temp entity.
	*	@param vecOrigin Origin.
	*	@param model Model to use.
	*	@return Temporary entity, or null if no entity could be allocated.
	*/
	TEMPENTITY*	( *CL_TempEntAlloc )			( const float* vecOrigin, model_s* model );

	/**
	*	Allocates a low priority temp entity with no model.
	*	@param vecOrigin Origin.
	*	@return Temporary entity, or null if no entity could be allocated.
	*/
	TEMPENTITY*	( *CL_TempEntAllocNoModel )		( const float* vecOrigin );

	/**
	*	Allocates a high priority temp entity. High priority temp entities are never freed when a high priority entity is created.
	*	@param vecOrigin Origin.
	*	@param model Model to use.
	*	@return Temporary entity, or null if no entity could be allocated.
	*/
	TEMPENTITY*	( *CL_TempEntAllocHigh )		( const float* vecOrigin, model_s* model );

	/**
	*	@param vecOrigin Origin.
	*	@param model Model to use.
	*	@param high Whether this is a high or low priority entity. @see TempEntPriority
	*	@param callback Think callback.
	*	@return Temporary entity, or null if no entity could be allocated.
	*/
	TEMPENTITY*	( *CL_TentEntAllocCustom )		( const float* origin, model_s *model, int high, void (*callback) (struct tempent_s* ent, float frametime, float currenttime));

	/**
	*	Obsolete. Always zeroes out packed.
	*/
	void		( *R_GetPackedColor )			( short *packed, short color );

	/**
	*	Looks up the index of the given RGB color in the base palette.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@return Index, or -1 if it couldn't be found.
	*/
	short		( *R_LookupColor )				( unsigned char r, unsigned char g, unsigned char b );

	/**
	*	Removes all instances of a particular decal.
	*	@param textureIndex Points to the decal index in the array, not the actual texture index.
	*/
	void		( *R_DecalRemoveAll )			( int textureIndex );

	/**
	*	Projects a decal onto the given brush entity's model.
	*	@param textureIndex Decal to project.
	*	@param entity Index of the entity to project onto.
	*	@param modelIndex Index of the entity's model to project onto.
	*	@param vecPosition Position in the world to project at.
	*	@param flags Flags. TODO: figure these out. - Solokiller
	*	@param scale Scale.
	*/
	void		( *R_FireCustomDecal )			( int textureIndex, int entity, int modelIndex, const float* vecPosition, int flags, float scale );
};

extern efx_api_t efx;

#endif
