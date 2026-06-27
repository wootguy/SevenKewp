#include "hud.h"
#include "cl_util.h"
#include "pm_defs.h"
#include "pm_materials.h"
#include "eventscripts.h"
#include "ev_hldm.h"
#include "event_api.h"
#include "in_defs.h"
#include "wc_params.h"
#include "engine_pv.h"
#include "com_weapons.h"
#include "ev_custom.h"

void WC_EV_LocalSound(int sndIdx, int chan, int pitch, float vol, float attn, int panning, int flags, Vector* sndOri) {
	const char* soundPath = GetSoundByIndex(sndIdx);
	int entidx = 0;
	Vector origin;

	if (sndOri) {
		entidx = 0;
		origin = *sndOri;
		chan = CHAN_STATIC;
	}
	else {
		cl_entity_t* player = GetLocalPlayer();
		entidx = player->index;
		origin = player->origin;
	}

	if (!soundPath) {
		PRINTF("Bad sound index %d\n", sndIdx);
		return;
	}

	if (panning == 1) {
		// playing sounds in stereo sounds kind of cool but this method of panning is bad
		// and shifts ears as you turn around
		//origin = origin + right * -8;
		//idx = 0;

		vol *= 0.8f;
	}
	else if (panning == 2) {
		vol *= 0.8f;
	}

	gEngfuncs.pEventAPI->EV_PlaySound(entidx, origin, chan, soundPath, vol, attn, flags, pitch);
}

void WC_EV_EjectShell(WepEvt& evt, bool leftHand) {
	cl_entity_t* player = GetLocalPlayer();
	int entidx = player->index;
	Vector angles = gPlayerSim.v_angles;
	Vector forward, right, up;
	AngleVectors(angles, forward, right, up);

	Vector view_ofs;
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
	Vector origin = gPlayerSim.v_sim_org;

	Vector ShellVelocity;
	Vector ShellOrigin;

	float forwardScale = evt.ejectShell.position[0];
	float upScale = evt.ejectShell.position[1];
	float rightScale = evt.ejectShell.position[2];

	static event_args_s args;
	args.entindex = entidx;

	if (leftHand) {
		right = right * -1;
	}

	EV_GetDefaultShellInfo(&args, origin, gPlayerSim.v_sim_vel, ShellVelocity, ShellOrigin,
		forward, right, up, forwardScale, upScale, rightScale);

	if (evt.ejectShell.hasVel) {
		Vector newForward = evt.ejectShell.vel[0] * forward;
		Vector newUp = evt.ejectShell.vel[1] * up;
		Vector newRight = evt.ejectShell.vel[2] * right;
		Vector vel = newForward + newUp + newRight;

		float speedMult = 5;

		if (evt.ejectShell.hasRand) {
			Vector dir = vel.Normalize();
			int r = evt.ejectShell.dirRand;
			dir.x += gEngfuncs.pfnRandomFloat(-r, r) * 0.01f;
			dir.y += gEngfuncs.pfnRandomFloat(-r, r) * 0.01f;
			dir.z += gEngfuncs.pfnRandomFloat(-r, r) * 0.01f;
			speedMult += gEngfuncs.pfnRandomFloat(0, evt.ejectShell.speedRand);
			ShellVelocity = gPlayerSim.v_sim_vel + dir * vel.Length() * speedMult;
		}
		else {
			ShellVelocity = gPlayerSim.v_sim_vel + vel * speedMult;
		}
	}

	EV_EjectBrass(ShellOrigin, ShellVelocity, angles[YAW], evt.ejectShell.model, evt.ejectShell.sound);
}

Vector& WC_EV_GetRecoil() {
	return gPlayerSim.ev_punchangle;
}

void WC_EV_Recoil(Vector recoil, int ops[3]) {
	Vector viewAngles;
	gEngfuncs.GetViewAngles(viewAngles);

	bool shouldSetView = false;

	for (int i = 0; i < 3; i++) {
		switch (ops[i]) {
		case WC_RECOIL_APPLY_PUNCH_SET:
			gPlayerSim.ev_punchangle[i] = recoil[i];
			break;
		case WC_RECOIL_APPLY_PUNCH_ADD:
			gPlayerSim.ev_punchangle[i] += recoil[i];
			break;
		case WC_RECOIL_APPLY_ROTATE:
			viewAngles[i] += recoil[i];
			shouldSetView = true;
			break;
		}
	}

	if (shouldSetView)
		gEngfuncs.SetViewAngles(viewAngles);
}

void WC_EV_WepAnim(WepEvt& evt, int wepid, int animIdx) {
	gEngfuncs.pEventAPI->EV_WeaponAnimation(animIdx, GetCustomWeaponBody(wepid));
}

void WC_EV_Dlight(WepEvt& evt, Vector pos) {
	dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
	dl->origin = pos;
	dl->radius = evt.dlight.radius * 10;
	dl->color.r = evt.dlight.color.r;
	dl->color.g = evt.dlight.color.g;
	dl->color.b = evt.dlight.color.b;
	dl->decay = evt.dlight.decayRate * 10;
	dl->die = gEngfuncs.GetClientTime() + evt.dlight.life * 0.1f;
}

pmtrace_t WC_EV_FireBullets(float spreadX, float spreadY, bool showTracer, int tracerColor,
	bool gunshotDecal, bool textureSound, bool particles, int iShot, int iDamage, float maxRange)
{
	pmtrace_t tr;

	cl_entity_t* player = GetLocalPlayer();
	int idx = player->index;

	Vector origin = gPlayerSim.v_sim_org;
	Vector view_ofs;
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);

	Vector vecSrc = origin + view_ofs;
	Vector forward, right, up;
	Vector angles = gPlayerSim.v_angles + gPlayerSim.sv_punchangle;
	AngleVectors(angles, forward, right, up);

	Vector vecDir = forward + spreadX * right + spreadY * up;
	Vector vecEnd = vecSrc + vecDir * maxRange;

	//gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
	//gEngfuncs.pEventAPI->EV_PushPMStates();
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

	// TODO: tracers do not match the server when quickly moving the mouse or during rapid fire
	// gEngfuncs.GetViewAngles() is more accurate for beams, so try using that here
	/*
	gEngfuncs.Con_Printf("Forward: %.4f %.4f %.4f\n", forward.x, forward.y, forward.z);
	gEngfuncs.Con_Printf("VecDir: %.4f %.4f %.4f\n", vecDir.x, vecDir.y, vecDir.z);
	int m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");
	gEngfuncs.pEfxAPI->R_BeamPoints(vecSrc, vecEnd, m_iBeam, 5, 1, 0, 1, 0, 0, 0, 0, 1, 1);
	*/

	if (showTracer) {
		Vector offset(0, 0, -4);
		Vector vecTracerSrc = vecSrc + offset + right * 2 + forward * 16;
		EV_CreateTracer(vecTracerSrc, tr.endpos, tracerColor);
	}

	// do damage, paint decals
	if (tr.fraction != 1.0) {
		EV_HLDM_DecalGunshot(&tr, BULLET_PLAYER_9MM, gunshotDecal, textureSound, particles);
	}


	float splashSize = 0.3f;
	if (iDamage > 50) {
		splashSize = 0.5f;
	}
	else if (iDamage > 8) {
		splashSize = 0.4f;
	}

	UTIL_WaterSplashTrace(vecSrc, tr.endpos, splashSize, iShot % 2 ? 2 : 0, NULL);

	//gEngfuncs.pEventAPI->EV_PopPMStates();

	return tr;
}

cl_entity_t* WC_GetPlayer() {
	return GetLocalPlayer();
}

Vector WC_GetGunPosition() {
	Vector view_ofs;
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
	return gPlayerSim.v_sim_org + view_ofs;
}

Vector WC_GetAim(float spreadX, float spreadY) {
	Vector angles, forward, right, up;
	gEngfuncs.GetViewAngles((float*)angles);
	AngleVectors(angles, forward, right, up);
	return forward + spreadX * right + spreadY * up;
}
