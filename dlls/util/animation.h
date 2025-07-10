/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#ifndef ANIMATION_H
#define ANIMATION_H

#define ACTIVITY_NOT_AVAILABLE		-1

#ifndef MONSTEREVENT_H
#include "monsterevent.h"
#endif

enum PLAYER_MODEL_ANIM_SET {
	PMODEL_ANIMS_HALF_LIFE,			// standard valve player model animations
	PMODEL_ANIMS_HALF_LIFE_COOP,	// sven co-op 5.x model ported to this mod (preserves HL anim order)
	PMODEL_ANIMS_SVEN_COOP_5,		// standard sven co-op 5.x player model animations
};

// will load from memory if cached, otherwise loads the model data from disk
EXPORT studiohdr_t* GetPlayerModelPtr(const char* name, int& len);

EXPORT PLAYER_MODEL_ANIM_SET GetPlayerModelAnimSet(studiohdr_t* mdl);

EXPORT void ClearPlayerModelCache();

EXPORT int IsSoundEvent( int eventNumber );

// Ranomly pick a sequence tagged with the given activity
EXPORT int LookupActivity( void *pmodel, entvars_t *pev, int activity );

// Pick the sequence with the heaviest random chance
EXPORT int LookupActivityHeaviest( void *pmodel, entvars_t *pev, int activity );

// Find the n'th sequence tagged with the given activity. If the offset is greater than
// the number of sequences tagged with this activity, then the last found sequence is returned.
EXPORT int LookupActivityWithOffset(void* pmodel, entvars_t* pev, int activity, int offset);

EXPORT bool ActivityHasEvent(void* pmodel, int activity, int event);

EXPORT int LookupSequence( void *pmodel, const char *label );
EXPORT void GetSequenceInfo( void *pmodel, entvars_t *pev, float *pflFrameRate, float *pflGroundSpeed );
EXPORT int GetSequenceFlags( void *pmodel, entvars_t *pev );
EXPORT int LookupAnimationEvents( void *pmodel, entvars_t *pev, float flStart, float flEnd );
EXPORT float SetController( void *pmodel, entvars_t *pev, int iController, float flValue );
EXPORT float SetBlending( void *pmodel, entvars_t *pev, int iBlender, float flValue );
EXPORT void GetEyePosition( void *pmodel, float *vecEyePosition );
EXPORT void SequencePrecache( void *pmodel, const char *pSequenceName );
EXPORT int FindTransition( void *pmodel, int iEndingAnim, int iGoalAnim, int *piDir );
EXPORT void SetBodygroup( void *pmodel, entvars_t *pev, int iGroup, int iValue );
EXPORT int GetBodygroup( void *pmodel, entvars_t *pev, int iGroup );
EXPORT int GetBodygroups( void *pmodel, entvars_t *pev );

EXPORT int GetAnimationEvent( void *pmodel, entvars_t *pev, MonsterEvent_t *pMonsterEvent, float flStart, float flEnd, int index );
EXPORT int ExtractBbox( void *pmodel, int sequence, float *mins, float *maxs );

// From /engine/studio.h
#define STUDIO_LOOPING		0x0001


#endif	//ANIMATION_H
