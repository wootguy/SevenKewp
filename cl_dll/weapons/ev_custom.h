#pragma once
#include "wc_events.h"
#include "pm_defs.h"

// custom weapon events

typedef struct cl_entity_s cl_entity_t;

void WC_EV_LocalSound(int sndIdx, int chan, int pitch, float vol, float attn, int panning, int flags, Vector* sndOri);
void WC_EV_EjectShell(WepEvt& evt, bool leftHand);
void WC_EV_Recoil(Vector recoil, int ops[3]);
Vector& WC_EV_GetRecoil();
void WC_EV_WepAnim(WepEvt& evt, int wepid, int animIdx);
void WC_EV_Dlight(WepEvt& evt, Vector pos);
pmtrace_t WC_EV_FireBullets(float spreadX, float spreadY, bool showTracer, int tracerColor, bool gunshotDecal, bool textureSound, bool particles, int iShot, int iDamage, float maxRange);
cl_entity_t* WC_GetPlayer();
Vector WC_GetGunPosition();
Vector WC_GetAim(float spreadX, float spreadY);