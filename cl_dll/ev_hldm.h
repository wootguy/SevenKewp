//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( EV_HLDMH )
#define EV_HLDMH
#include "wc_events.h"
#include "../common/bullet.h"

enum glock_e {
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

enum shotgun_e {
	SHOTGUN_IDLE = 0,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};

enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};

enum python_e {
	PYTHON_IDLE1 = 0,
	PYTHON_FIDGET,
	PYTHON_FIRE1,
	PYTHON_RELOAD,
	PYTHON_HOLSTER,
	PYTHON_DRAW,
	PYTHON_IDLE2,
	PYTHON_IDLE3
};

#define	GAUSS_PRIMARY_CHARGE_VOLUME	256// how loud gauss is while charging
#define GAUSS_PRIMARY_FIRE_VOLUME	450// how loud gauss is when discharged

enum gauss_e {
	GAUSS_IDLE = 0,
	GAUSS_IDLE2,
	GAUSS_FIDGET,
	GAUSS_SPINUP,
	GAUSS_SPIN,
	GAUSS_FIRE,
	GAUSS_FIRE2,
	GAUSS_HOLSTER,
	GAUSS_DRAW
};

void EV_HLDM_GunshotDecalTrace( pmtrace_t *pTrace, char *decalName, bool playSound );
void EV_HLDM_DecalGunshot( pmtrace_t *pTrace, int iBulletType, bool playSound );
int EV_HLDM_CheckTracer( int idx, float *vecSrc, float *end, float *forward, float *right, int iBulletType, int iTracerFreq, int *tracerCount );
void EV_HLDM_FireBullets( int idx, float *forward, float *right, float *up, int cShots, float *vecSrc, float *vecDirShooting, float flDistance, int iBulletType, int iTracerFreq, int *tracerCount, float flSpreadX, float flSpreadY );
void EV_EgonFlareCallback(struct tempent_s* ent, float frametime, float currenttime);
void EV_HLDM_GunshotDecalEffects(Vector pos, bool playSound);
void EV_LaserOn(int dotSprite, float dotSz, int beamSprite, float beamWidth, int beamAttachment);
void EV_LaserOff();

// custom weapon events
void WC_EV_LocalSound(int sndIdx, int chan, int pitch, float vol, float attn, int panning, int flags, Vector* sndOri);
void WC_EV_EjectShell(WepEvt& evt, bool leftHand);
void WC_EV_PunchAngle(WepEvt& evt, int seed, float attackTime);
void WC_EV_WepAnim(WepEvt& evt, int wepid, int animIdx);
void WC_EV_Dlight(WepEvt& evt, Vector pos);
pmtrace_t WC_EV_FireBullets(float spreadX, float spreadY, bool showTracer, int tracerColor, bool gunshotDecal, bool textureSound, int iShot, int iDamage);
cl_entity_t* WC_GetPlayer();
Vector WC_GetGunPosition();
Vector WC_GetAim(float spreadX, float spreadY);

#endif // EV_HLDMH