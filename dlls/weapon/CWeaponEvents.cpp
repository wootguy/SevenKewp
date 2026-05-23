#include "CWeaponCustom.h"

int CWeaponEvents::m_tracerCount[32];
TraceResult g_traces[256];

// convert a client-side trace struct to the server-side kind
#ifdef CLIENT_DLL
WcTrace ConvertTrace(pmtrace_t tr) {
	WcTrace out;

	out.fAllSolid = tr.allsolid;
	out.fStartSolid = tr.startsolid;
	out.fInOpen = tr.inopen;
	out.fInWater = tr.inwater;
	out.flFraction = tr.fraction;
	out.vecEndPos = tr.endpos;
	out.flPlaneDist = tr.plane.dist;
	out.vecPlaneNormal = tr.plane.normal;
	out.iHitgroup = tr.hitgroup;
	out.pHit = gEngfuncs.pEventAPI->EV_IndexFromTrace(&tr);

	return out;
}
#else
WcTrace ConvertTrace(TraceResult tr) {
	WcTrace out;

	out.fAllSolid = tr.fAllSolid;
	out.fStartSolid = tr.fStartSolid;
	out.fInOpen = tr.fInOpen;
	out.fInWater = tr.fInWater;
	out.flFraction = tr.flFraction;
	out.vecEndPos = tr.vecEndPos;
	out.flPlaneDist = tr.flPlaneDist;
	out.vecPlaneNormal = tr.vecPlaneNormal;
	out.iHitgroup = tr.iHitgroup;
	out.pHit = tr.pHit ? ENTINDEX(tr.pHit) : 0;

	return out;
}
#endif

bool WcBeam::isFree() {
#ifdef CLIENT_DLL
	return !pBeam || pBeam->die < gEngfuncs.GetClientTime();
#else
	return !h_beam.GetEntity();
#endif
}

void WcSprite::Kill() {
#ifdef CLIENT_DLL
	if (pSprite) {
		pSprite->die = gEngfuncs.GetClientTime();
		pSprite->callback = &EV_EgonFlareCallback;
		pSprite->fadeSpeed = 2.0;			// fade out will take 0.5 sec
		pSprite->tentOffset.x = 10.0;		// scaling speed per second
		pSprite->tentOffset.y = 0.1;			// min time between two scales
		pSprite->tentOffset.z = pSprite->die;	// the last callback run time
		pSprite->flags = FTENT_FADEOUT | FTENT_CLIENTCUSTOM;
		pSprite = NULL;
	}
#else
	if (h_sprite.GetEntity()) {
		CSprite* spr = (CSprite*)h_sprite.GetEntity();
		spr->Expand(10, 500);
		h_sprite = NULL;
	}
#endif
}

bool WcSprite::IsAlive() {
#ifdef CLIENT_DLL
	return pSprite;
#else
	return h_sprite.GetEntity();
#endif
}



void CWeaponEvents::ProcessEvents(int trigger, int triggerArg, bool leftHand, bool akimboFire, int clipLeft, WcTrace* tr) {
#ifdef CLIENT_DLL
	if (!g_runfuncs)
		return;
#endif

	CBasePlayer* m_pPlayer = m_weapon->GetPlayer();
	if (!m_pPlayer)
		return;

	for (int i = 0; i < m_weapon->params.numEvents; i++) {
		WepEvt& evt = m_weapon->params.events[i];

		if (evt.trigger != trigger)
			continue;

		bool argMatch = true;
		switch (trigger) {

		case WC_TRIG_PRIMARY:
		case WC_TRIG_SECONDARY:
		case WC_TRIG_TERTIARY:
		case WC_TRIG_RELOAD:
		case WC_TRIG_RELOAD_EMPTY:
		case WC_TRIG_RELOAD_NOT_EMPTY:
		case WC_TRIG_DEPLOY:
			argMatch = evt.triggerArg == WC_TRIG_SHOOT_ARG_ALWAYS || triggerArg == evt.triggerArg;
			break;
		case WC_TRIG_PRIMARY_CLIPSIZE:
		case WC_TRIG_IMPACT:
			argMatch = triggerArg == evt.triggerArg;
			break;
		case WC_TRIG_PRIMARY_CLIP_SP:
		case WC_TRIG_SECONDARY_CLIP_SP:
			switch (evt.triggerArg) {
			case WC_TRIG_CLIP_ARG_ODD:
				argMatch = triggerArg % 2 != 0;
				break;
			case WC_TRIG_CLIP_ARG_EVEN:
				argMatch = triggerArg % 2 == 0;
				break;
			case WC_TRIG_CLIP_ARG_NOT_EMPTY:
				argMatch = clipLeft > 0;
				break;
			}
			break;
		default:
			break;
		}

		if (!argMatch)
			continue;

		if (evt.delay == 0) {
			PlayEvent(i, leftHand, akimboFire, tr);

			if (evt.evtType == WC_EVT_BULLETS && evt.bullets.burstDelay) {
				float burstDelay = 0;
				int additionalBullets = evt.bullets.count - 1;
				if (clipLeft < 0) // clip went negative due to exceeding cost for a burst
					additionalBullets += clipLeft;
				for (int k = 0; k < additionalBullets; k++) {
					burstDelay += evt.bullets.burstDelay * 0.001f;
					QueueDelayedEvent(i, m_weapon->WallTime() + burstDelay, leftHand, akimboFire, tr);
				}
			}
		}
		else {
			QueueDelayedEvent(i, m_weapon->WallTime() + evt.delay * 0.001f, leftHand, akimboFire, tr);
		}
	}
}

void CWeaponEvents::QueueDelayedEvent(int eventIdx, float fireTime, bool leftHand, bool akimboFire, WcTrace* tr) {
	for (int i = 0; i < WC_SERVER_EVENT_QUEUE_SZ; i++) {
		WcDelayEvent& qevt = eventQueue[i];

		// find an empty slot
		if (qevt.fireTime == 0) {
			qevt.eventIdx = eventIdx;
			qevt.fireTime = fireTime;
			qevt.leftHand = leftHand;
			qevt.akimboFire = akimboFire;

			if (tr) {
				qevt.tr = *tr;
			}
			else {
				memset(&qevt.tr, 0, sizeof(WcTrace));
			}
			//CLALERT("Queue event %d\n", eventIdx);
			return;
		}
	}

	CBasePlayer* m_pPlayer = m_weapon->GetPlayer();
	if (!m_pPlayer)
		return;
	ALERT(at_console, "Server event queue is full for %s on player %s\n", STRING(m_weapon->pev->classname), m_pPlayer->DisplayName());
}

void CWeaponEvents::PlayEvent_Bullets(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand, bool akimboFire) {
	Vector spread(SPREAD_TO_FLOAT(evt.bullets.accuracy[0]), SPREAD_TO_FLOAT(evt.bullets.accuracy[1]), 0);

	if (evt.bullets.flags & FL_WC_BULLETS_DYNAMIC_SPREAD) {
		spread = spread * GetCurrentAccuracyMultiplier();
	}

	if (evt.bullets.flashSz) {
		WepEvt flash = WepEvt();
		flash.muzzleFlash.brightness = evt.bullets.flashSz;
		PlayEvent_MuzzleFlash(flash, m_pPlayer);
	}

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	bool isPredicted = m_weapon->IsPredicted();
	BULLET_PREDICTION predFlag = isPredicted ? BULLETPRED_EVENTLESS : BULLETPRED_NONE;

	float damage = evt.bullets.damage * GetChargeMult(evt, FL_WC_CHARGE_DAMAGE);

	lagcomp_begin(m_pPlayer);
	Vector vecDir = m_pPlayer->FireBulletsPlayer(evt.bullets.count, vecSrc, vecAiming, spread, 8192,
		BULLET_PLAYER_9MM, evt.bullets.tracerFreq, damage, m_pPlayer->pev,
		m_pPlayer->random_seed, g_traces, predFlag);
	lagcomp_end();

	int attackIdx = m_weapon->GetAttackIdx(evt);

#ifdef CLIENT_DLL
	int eidx = 0;
#else
	int eidx = m_pPlayer->entindex() - 1;
#endif

	bool showTracer = CheckTracer(eidx, vecSrc, vecDir, gpGlobals->v_right, evt.bullets.tracerFreq);

#ifdef CLIENT_DLL
	bool decal = !(evt.bullets.flags & FL_WC_BULLETS_NO_DECAL);
	bool texSound = !(evt.bullets.flags & FL_WC_BULLETS_NO_SOUND);

	for (ULONG iShot = 1; iShot <= evt.bullets.count; iShot++)
	{
		//Use player's random seed.
		// get circular gaussian spread
		int r = m_pPlayer->random_seed;
		float x = UTIL_SharedRandomFloat(r + iShot, -0.5, 0.5) + UTIL_SharedRandomFloat(r + (1 + iShot), -0.5, 0.5);
		float y = UTIL_SharedRandomFloat(r + (2 + iShot), -0.5, 0.5) + UTIL_SharedRandomFloat(r + (3 + iShot), -0.5, 0.5);
		float z = x * x + y * y;

		bool playTexSound = texSound && iShot < 6; // don't stack too many sounds
		pmtrace_t tr = WC_EV_FireBullets(x * spread.x, y * spread.y, showTracer, evt.bullets.tracerColor,
			decal, playTexSound, iShot, evt.bullets.damage);

		WcTrace evTrace = ConvertTrace(tr);
		ProcessEvents(WC_TRIG_IMPACT, GetImpactArg(attackIdx, true, true), false, false, 0, &evTrace);
	}

	if (evt.bullets.flashSz)
		EV_MuzzleFlash();
#else

	for (int i = 0; i < evt.bullets.count; i++) {
		WcTrace evTrace = ConvertTrace(g_traces[i]);
		ProcessEvents(WC_TRIG_IMPACT, GetImpactArg(attackIdx, true, true), false, false, 0, &evTrace);
		m_weapon->AttackTrace(m_pPlayer, attackIdx, vecSrc, g_traces[i]);
	}

	if (showTracer) {
		for (int i = 1; i < gpGlobals->time; i++) {
			CBasePlayer* listener = UTIL_PlayerByIndex(i);

			if (!listener) {
				continue;
			}

			if ((m_pPlayer != listener || !isPredicted) && m_pPlayer->InPAS(listener->edict())) {
				for (int k = 0; k < evt.bullets.count; k++) {
					UTIL_Tracer(vecSrc, g_traces[k].vecEndPos, evt.bullets.tracerColor, MSG_ONE_UNRELIABLE, listener->edict());
				}
			}
		}
	}
#endif

	int akimboArg = m_weapon->IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
	ProcessEvents(WC_TRIG_BULLET_FIRED, akimboArg, leftHand, akimboFire);
}

void CWeaponEvents::PlayEvent_Beam(WepEvt& evt, CBasePlayer* m_pPlayer) {
	WcBeam* wcbeam;
	if (evt.beam.id == 0)
		wcbeam = AllocBeam();
	else
		wcbeam = &m_beams[V_min(MAX_WC_BEAMS - 1, evt.beam.id)];

	if (!wcbeam)
		return;

	wcbeam->evt = evt;

	bool newBeam = evt.beam.id == 0 || wcbeam->isFree();

	Vector endPos;
	if (newBeam) {
		endPos = BeamAttack(*wcbeam, m_pPlayer);
		wcbeam->creationTime = gpGlobals->time;
		if (evt.beam.hasImpactSprite)
			m_beamImpactSprite.Kill();
	}
	else {
		if (evt.beam.hasImpactSprite)
			m_beamImpactSprite.killTime = evt.beam.life ? gpGlobals->time + evt.beam.life : 0;
	}

#ifdef CLIENT_DLL
	float life = evt.beam.life ? evt.beam.life * 0.001f : 99999;

	if (newBeam) {
		// create new beam
		cl_entity_t* player = WC_GetPlayer();
		int idx = player->index;

		float r = (evt.beam.color.r / 255.0f);
		float g = (evt.beam.color.g / 255.0f);
		float b = (evt.beam.color.b / 255.0f);
		float a = (evt.beam.color.a / 255.0f);

		BEAM* beam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | (0x1000 * evt.beam.attachment),
			endPos, evt.beam.sprite, life, evt.beam.width / 10.0f, evt.beam.noise / 100.0f, a, evt.beam.scrollRate,
			0, 0, r, g, b);

		if (evt.beam.flags & FL_WC_BEAM_SPIRAL) beam->flags |= BEAM_FSINE;
		if (evt.beam.flags & FL_WC_BEAM_OPAQUE) beam->flags |= BEAM_FSOLID;
		if (evt.beam.flags & FL_WC_BEAM_SHADEIN) beam->flags |= BEAM_FSHADEIN;
		if (evt.beam.flags & FL_WC_BEAM_SHADEOUT) beam->flags |= BEAM_FSHADEOUT;

		wcbeam->pBeam = beam;
		wcbeam->pBeam->die = gEngfuncs.GetClientTime() + life;
		wcbeam->pBeam->target = endPos;

		if (evt.beam.hasImpactSprite) {
			RGBA c = evt.beam.impactSpriteColor;

			TEMPENTITY* pFlare = gEngfuncs.pEfxAPI->R_TempSprite(endPos, Vector(0, 0, 0),
				evt.beam.impactSpriteScale * 0.1f, evt.beam.impactSprite, kRenderGlow,
				kRenderFxNoDissipation, c.a / 255.0f, 99999, FTENT_SPRCYCLE | FTENT_PERSIST);

			m_beamImpactSprite.creationTime = gpGlobals->time;
			m_beamImpactSprite.pSprite = pFlare;
			m_beamImpactSprite.beamId = evt.beam.id ? evt.beam.id : -1;
			m_beamImpactSprite.killTime = evt.beam.life ? gpGlobals->time + evt.beam.life : 0;
		}
	}
	else {
		wcbeam->pBeam->die = gEngfuncs.GetClientTime() + life;
	}
#else
	if (newBeam) {
		// create new beam
		CBeam* beam = CBeam::BeamCreate(INDEX_MODEL(evt.beam.sprite), evt.beam.width);
		beam->PointEntInit(endPos, m_pPlayer->entindex());
		beam->SetEndAttachment(evt.beam.attachment);
		beam->SetNoise(evt.beam.noise);
		beam->SetColor(evt.beam.color.r, evt.beam.color.g, evt.beam.color.b);
		beam->SetBrightness(evt.beam.color.a);
		beam->SetScrollRate(evt.beam.scrollRate);

		int flags = 0;
		if (evt.beam.flags & FL_WC_BEAM_SPIRAL) flags |= BEAM_FSINE;
		if (evt.beam.flags & FL_WC_BEAM_OPAQUE) flags |= BEAM_FSOLID;
		if (evt.beam.flags & FL_WC_BEAM_SHADEIN) flags |= BEAM_FSHADEIN;
		if (evt.beam.flags & FL_WC_BEAM_SHADEOUT) flags |= BEAM_FSHADEOUT;
		beam->SetFlags(flags);

		if (evt.beam.life)
			beam->LiveForTime(evt.beam.life * 0.001f);
		beam->m_hidePlayers = PLRBIT(m_pPlayer->edict());
		wcbeam->h_beam = beam;

		if (evt.beam.hasImpactSprite) {
			CSprite* spr = CSprite::SpriteCreate(INDEX_MODEL(evt.beam.impactSprite), endPos, TRUE);
			RGBA c = evt.beam.impactSpriteColor;
			spr->SetColor(c.r, c.g, c.b);
			spr->SetBrightness(c.a);
			spr->SetScale(evt.beam.impactSpriteScale);
			spr->pev->rendermode = kRenderGlow;
			spr->pev->renderfx = kRenderFxNoDissipation;
			spr->pev->framerate = evt.beam.impactSpriteFps;
			spr->m_hidePlayers = PLRBIT(m_pPlayer->edict());

			m_beamImpactSprite.creationTime = gpGlobals->time;
			m_beamImpactSprite.beamId = evt.beam.id ? evt.beam.id : -1;
			m_beamImpactSprite.killTime = evt.beam.life ? gpGlobals->time + evt.beam.life : 0;
			m_beamImpactSprite.h_sprite = spr;
		}
	}
	else {
		// update existing beam
		CBeam* beam = (CBeam*)wcbeam->h_beam.GetEntity();

		if (evt.beam.life)
			beam->LiveForTime(evt.beam.life * 0.1f);
		else {
			beam->SetThink(NULL);
			beam->pev->nextthink = 0;
		}
	}
#endif
}

void CWeaponEvents::PlayEvent_Projectile(WepEvt& evt, CBasePlayer* m_pPlayer) {
#ifndef CLIENT_DLL
	MAKE_VECTORS(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	float x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
	float y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);

	Vector vForward = gpGlobals->v_forward;
	Vector vRight = gpGlobals->v_right;
	Vector vUp = gpGlobals->v_up;
	Vector vecSpread(SPREAD_TO_FLOAT(evt.proj.accuracy[0]), SPREAD_TO_FLOAT(evt.proj.accuracy[1]), 0);

	Vector vecDir = vForward +
		x * vecSpread.x * vRight +
		y * vecSpread.y * vUp;

	// Get amount of player velocity to add to projectile.
	Vector inf = evt.proj.player_vel_inf;
	Vector pvel = m_pPlayer->pev->velocity;
	pvel = vRight * DotProduct(vRight, pvel) * inf.x +
		vUp * DotProduct(vUp, pvel) * inf.y +
		vForward * DotProduct(vForward, pvel) * inf.z;

	Vector dir = evt.proj.dir;
	Vector projectile_velocity = pvel +
		dir.x * evt.proj.speed * vRight +
		dir.y * evt.proj.speed * Vector(0, 0, 1) +
		dir.z * evt.proj.speed * vecDir;

	Vector offsetOpts = evt.proj.offset;
	Vector ofs = vRight * offsetOpts.x + vForward * offsetOpts.y + vUp * offsetOpts.z;
	Vector projectile_ori = m_pPlayer->GetGunPosition() + ofs;
	Vector projectile_dir_angles = UTIL_VecToAngles(projectile_velocity.Normalize());
	float grenadeTime = evt.proj.life != 0 ? evt.proj.life / 1000.0f : 3.5f; // timed grenades only
	//if (state.active_opts.windup_time > 0)
	//	grenadeTime = Math.max(0, grenadeTime - (g_Engine.time - state.windupStart));

	CBaseEntity* shootEnt = NULL;
	switch (evt.proj.type) {
	case WC_PROJECTILE_ARGRENADE:
		shootEnt = CGrenade::ShootContact(m_pPlayer->pev, projectile_ori, projectile_velocity);
		break;
	case WC_PROJECTILE_BANANA:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_BANANA Not implemented\n");
		break;
	case WC_PROJECTILE_BOLT:
		shootEnt = CCrossbowBolt::BoltCreate();
		shootEnt->pev->origin = projectile_ori;
		shootEnt->pev->angles = m_pPlayer->pev->v_angle;
		shootEnt->pev->owner = m_pPlayer->edict();

		if (m_pPlayer->pev->waterlevel == 3)
		{
			shootEnt->pev->velocity = vecDir * BOLT_WATER_VELOCITY;
			shootEnt->pev->speed = BOLT_WATER_VELOCITY;
		}
		else
		{
			shootEnt->pev->velocity = vecDir * BOLT_AIR_VELOCITY;
			shootEnt->pev->speed = BOLT_AIR_VELOCITY;
		}
		shootEnt->pev->avelocity.z = 10;
		break;
	case WC_PROJECTILE_HVR:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_HVR Not implemented\n");
		break;
	case WC_PROJECTILE_SHOCK:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_SHOCK Not implemented\n");
		break;
	case WC_PROJECTILE_HORNET:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_HORNET Not implemented\n");
		break;
	case WC_PROJECTILE_DISPLACER:
		shootEnt = CDisplacerBall::CreateDisplacerBall(projectile_ori, projectile_dir_angles, m_pPlayer);
		break;
	case WC_PROJECTILE_GRENADE:
		shootEnt = CGrenade::ShootTimed(m_pPlayer->pev, projectile_ori, projectile_velocity, grenadeTime);
		break;
	case WC_PROJECTILE_MORTAR:
		shootEnt = ShootMortar(m_pPlayer->edict(), projectile_ori, projectile_velocity);
		break;
	case WC_PROJECTILE_RPG:
		shootEnt = CRpgRocket::CreateRpgRocket(projectile_ori, m_pPlayer->pev->v_angle, m_pPlayer, NULL);
		break;
	case WC_PROJECTILE_WEAPON:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_WEAPON Not implemented\n");
		break;
	case WC_PROJECTILE_TRIPMINE: {
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_TRIPMINE Not implemented\n");
		/*
		// assumes MakeVectors was already called
		Vector vecSrc = m_pPlayer->GetGunPosition();

		// Find a good tripmine location
		TraceResult tr;
		Vector vecEnd = vecSrc + vForward * state.active_opts.max_range;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);
		if (tr.flFraction >= 1.0 && state.active_opts.pev.spawnflags & FL_SHOOT_IF_NOT_MISS != 0)
		{
			state.abortAttack = true;
		}
		else
		{
			Vector tripOri = tr.vecEndPos + tr.vecPlaneNormal * 8;
			Vector angles = UTIL_VecToAngles(tr.vecPlaneNormal);
			angles.x *= -1; // not sure why hlsdk doesn't do this
			const char* tripClass = evt.proj.entity_class ? STRING(evt.proj.entity_class) : "monster_tripmine";
			shootEnt = ShootCustomProjectile(state, tripClass, tripOri, projectile_velocity, angles);
		}
		*/
		break;
	}
	case WC_PROJECTILE_CUSTOM: {
		const char* clazz = evt.proj.entity_class ? STRING(evt.proj.entity_class) : "custom_projectile";
		shootEnt = CBaseEntity::Create(clazz, projectile_ori, projectile_dir_angles, false);
		shootEnt->pev->velocity = projectile_velocity;
		shootEnt->pev->owner = m_pPlayer->edict();

		CProjectileCustom* cproj = shootEnt->MyProjectileCustomPtr();
		if (cproj) {
			cproj->world_event = (WeaponCustomProjectileAction)evt.proj.world_event;
			cproj->monster_event = (WeaponCustomProjectileAction)evt.proj.monster_event;
			cproj->air_friction = evt.proj.air_friction;
			cproj->water_friction = evt.proj.water_friction;
			cproj->damage = evt.proj.damage * GetChargeMult(evt, FL_WC_CHARGE_DAMAGE);
			cproj->damageType = evt.proj.damageBits;
			cproj->expire_time = evt.proj.life ? gpGlobals->time + evt.proj.life / 1000.0f : 0;
			cproj->pev->movetype = evt.proj.gravity != 0 ? MOVETYPE_BOUNCE : MOVETYPE_BOUNCEMISSILE;
			cproj->flags = evt.proj.flags;
			cproj->move_snd = evt.proj.move_snd;

			if (!evt.proj.model) {
				cproj->pev->rendermode = 1; // don't render the no-precache model
			}

			float size = evt.proj.size;
			cproj->pev->mins = Vector(-size, -size, -size);
			cproj->pev->maxs = Vector(size, size, size);

			cproj->Configure(m_pPlayer, m_weapon, evt);
		}

		edict_t* ed = shootEnt->edict();
		DispatchSpawnGame(ed);
		shootEnt = CBaseEntity::Instance(ed);
		break;
	}
	case WC_PROJECTILE_OTHER: {
		shootEnt = CBaseEntity::Create(STRING(evt.proj.entity_class), projectile_ori, projectile_dir_angles, false);
		shootEnt->pev->velocity = projectile_velocity;
		shootEnt->pev->owner = m_pPlayer->edict();
		edict_t* ed = shootEnt->edict();
		DispatchSpawnGame(ed);
		shootEnt = CBaseEntity::Instance(ed);
		break;
	}
	default:
		ALERT(at_error, "WeaponCustom: Unknown projectile type %d\n", evt.proj.type);
		break;
	}


	if (shootEnt)
	{
		AddWaterPhysicsEnt(shootEnt, 1, 0);

		if (evt.proj.follow_mode != WC_PROJ_FOLLOW_NONE)
		{
			//EHANDLE h_plr = m_pPlayer->edict();
			//EHANDLE h_proj = shootEnt->edict();
			//float dur = evt.proj.follow_time[1];
			ALERT(at_error, "WeaponCustom: Projectile follow mode not implemented\n");
			//g_Scheduler.SetTimeout("projectile_follow_aim", evt.proj.follow_time[0], h_plr, h_proj, @state.active_opts, dur);
		}

		if (evt.proj.model) {
			SET_MODEL(shootEnt->edict(), INDEX_MODEL(evt.proj.model));
		}

		//EHANDLE mdlHandle = shootEnt->edict();
		EHANDLE sprHandle;

		CProjectileCustom* cproj = shootEnt->MyProjectileCustomPtr();

		// TODO: Kill this when follow target dies (and its not a custom entity)
		if (evt.proj.sprite)
		{
			if (cproj) {
				Vector ori = shootEnt->pev->origin;
				RGBA c = evt.proj.sprite_color;
				CBaseEntity* spr = CBaseEntity::Create("env_sprite", ori, g_vecZero, false);
				spr->pev->model = evt.proj.sprite;
				spr->pev->rendermode = 5;
				spr->pev->renderamt = c.a;
				spr->pev->rendercolor = c.ToVector();
				spr->pev->scale = evt.proj.sprite_scale;
				spr->pev->movetype = MOVETYPE_FOLLOW;
				spr->pev->aiment = shootEnt->edict();
				spr->pev->skin = shootEnt->entindex();
				spr->pev->body = 0; // attachement point
				edict_t* ed = spr->edict();
				DispatchSpawnGame(ed);
				cproj->spriteAttachment = sprHandle = ed;
			}
			else {
				ALERT(at_error, "Projectile sprite not implemented for non-custom projectiles\n");
			}

		}

		// attach a trail
		if (evt.proj.trail_spr) {
			UTIL_BeamFollow(shootEnt->entindex(), MODEL_INDEX(STRING(evt.proj.trail_spr)), evt.proj.trail_life,
				evt.proj.trail_width, evt.proj.trail_color);
		}

		if (evt.proj.life && evt.proj.type != WC_PROJECTILE_CUSTOM)
			ALERT(at_error, "WeaponCustom: Non-custom projectile life not implemented\n");

		/*
		if (evt.proj.life > 0)
			g_Scheduler.SetTimeout("killProjectile", options.life, mdlHandle, sprHandle, state.active_opts);

		if (state.c_wep !is null and (state.c_wep.settings.max_live_projectiles > 0 or state.c_wep.settings.pev.spawnflags & FL_WEP_WAIT_FOR_PROJECTILES != 0))
		{
			state.liveProjectiles++;
			MonitorProjectileLife(state, mdlHandle);
		}
		*/

		if (evt.proj.hasAvel)
			shootEnt->pev->avelocity = evt.proj.avel;

		// TODO: Allow setting projectile class and ally status
		int rel = m_pPlayer->IRelationship(shootEnt);
		bool isFriendly = rel == R_AL || rel == R_NO;
		if (shootEnt->IsMonster() && !isFriendly)
			shootEnt->SetClassification(CLASS_PLAYER_ALLY);

		// TODO: health set here
		shootEnt->pev->friction = 1.0f - evt.proj.elasticity;
		shootEnt->pev->gravity = evt.proj.gravity;
		shootEnt->pev->angles = shootEnt->pev->angles + *(Vector*)evt.proj.angles;

		if (!shootEnt->pev->gravity && shootEnt->pev->movetype == MOVETYPE_BOUNCE) {
			shootEnt->pev->gravity = FLT_MIN;
		}
	}

	// remove weapon from player if they threw it
	if (evt.proj.type == WC_PROJECTILE_WEAPON) {
		UTIL_Remove(m_weapon);
	}
#endif
}

void CWeaponEvents::PlayEvent_Kickback(WepEvt& evt, CBasePlayer* m_pPlayer) {
	float force = evt.kickback.pushForce * GetChargeMult(evt, FL_WC_CHARGE_KICKBACK);
	float backForce = (evt.kickback.back / 100.0f) * force;
	float rightForce = (evt.kickback.right / 100.0f) * force;
	float upForce = (evt.kickback.up / 100.0f) * force;
	float globalUpForce = (evt.kickback.globalUp / 100.0f) * force;
	static Vector globalUp = Vector(0, 0, 1);

#ifdef CLIENT_DLL
	Vector forward, right, up;
	gEngfuncs.pfnAngleVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle, forward, right, up);
	m_weapon->m_runningKickbackPred = 1;
	m_weapon->m_kickbackPredVel = (forward * -backForce) + (right * rightForce) + (up * upForce) + (globalUp * globalUpForce);
#else
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	m_pPlayer->pev->velocity = m_pPlayer->pev->velocity +
		gpGlobals->v_forward * -backForce +
		gpGlobals->v_right * rightForce +
		gpGlobals->v_up * upForce +
		globalUp * globalUpForce;
#endif
}

void CWeaponEvents::PlayEvent_SetGravity(WepEvt& evt, CBasePlayer* m_pPlayer) {
	// TODO: update monster effects code to use this, like with movespeed
	m_pPlayer->pev->gravity = evt.setGravity.gravity / 1000.0f;
}

void CWeaponEvents::PlayEvent_Sound(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand, bool akimboFire) {
	int channel = CHAN_STATIC;
	int pitch = 100;
	float volume = evt.idleSound.volume / 127.0f;
	float attn = ATTN_IDLE;
	int idx = evt.idleSound.sound;

	if (evt.evtType == WC_EVT_PLAY_SOUND) {
		idx = evt.playSound.sound;
		if (evt.playSound.additionalSounds.arrSz) {
			int rnd = UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, evt.playSound.additionalSounds.arrSz);
			if (rnd > 0) {
				idx = evt.playSound.additionalSounds.arr[rnd - 1];
			}
		}

		channel = evt.playSound.channel;
		pitch = UTIL_SharedRandomLong(m_pPlayer->random_seed, evt.playSound.pitchMin, evt.playSound.pitchMax);
		volume = evt.playSound.volume / 255.0f;
		attn = evt.playSound.attn / 64.0f;

		if (evt.playSound.flags & FL_WC_SOUND_CHARGE_PITCH)
			pitch = evt.playSound.pitchMin;

		switch (evt.playSound.aiVol) {
		case WC_AIVOL_QUIET:
			m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
			break;
		case WC_AIVOL_NORMAL:
			m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
			break;
		case WC_AIVOL_LOUD:
			m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
			break;
		default:
			break;
		}
	}

#ifdef CLIENT_DLL
	int panning = 0;
	if (akimboFire)
		panning = leftHand ? 1 : 2; // signal the event player to pan the audio

	WC_EV_LocalSound(idx, channel, pitch, volume, attn, panning, 0);
#else

	if (m_weapon->IsPredicted()) {
		uint32_t messageTargets = 0xffffffff & ~PLRBIT(m_pPlayer->edict());
		StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(idx), volume, attn,
			SND_FL_PREDICTED, pitch, m_pPlayer->pev->origin, messageTargets);
	}
	else {
		StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(idx), volume, attn,
			0, pitch, m_pPlayer->pev->origin, 0xffffffff);
	}

	if (evt.playSound.distantSound) {
		PLAY_DISTANT_SOUND(m_pPlayer->edict(), evt.playSound.distantSound);
	}
#endif
}

void CWeaponEvents::PlayEvent_EjectShell(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand) {
#ifdef CLIENT_DLL
	WC_EV_EjectShell(evt, leftHand);
#else
	Vector ori = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
	Vector vel = m_pPlayer->pev->velocity;
	Vector angles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

	Vector forward, right, up;
	UTIL_MakeVectorsPrivate(angles, forward, right, up);

	if (leftHand)
		right = right * -1;

	float forwardScale = evt.ejectShell.offset[0] * 0.01f;
	float upScale = evt.ejectShell.offset[1] * 0.01f;
	float rightScale = evt.ejectShell.offset[2] * 0.01f;

	float fR = RANDOM_FLOAT(50, 70);
	float fU = RANDOM_FLOAT(100, 150);

	Vector ShellVelocity = vel + right * fR + up * fU + forward * 25;
	Vector ShellOrigin = ori + up * upScale + forward * forwardScale + right * rightScale;

	if (evt.ejectShell.hasVel) {
		Vector newForward = evt.ejectShell.vel[0] * forward;
		Vector newUp = evt.ejectShell.vel[1] * up;
		Vector newRight = evt.ejectShell.vel[2] * right;
		Vector svel = newForward + newUp + newRight;

		float speedMult = 5;

		if (evt.ejectShell.hasRand) {
			Vector dir = svel.Normalize();
			int r = evt.ejectShell.dirRand;
			dir.x += RANDOM_FLOAT(-r, r) * 0.01f;
			dir.y += RANDOM_FLOAT(-r, r) * 0.01f;
			dir.z += RANDOM_FLOAT(-r, r) * 0.01f;
			speedMult += RANDOM_FLOAT(0, evt.ejectShell.speedRand);
			ShellVelocity = vel + dir * svel.Length() * speedMult;
		}
		else {
			ShellVelocity = vel + svel * speedMult;
		}
	}

	bool predicted = m_weapon->IsPredicted();

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr || (plr == m_pPlayer && predicted))
			continue;

		if (m_pPlayer->InPAS(plr->edict())) {
			EjectBrass(ShellOrigin, ShellVelocity, angles.y, evt.ejectShell.model, TE_BOUNCE_SHELL, plr->edict());
		}
	}
#endif
}

void CWeaponEvents::PlayEvent_PunchAngle(WepEvt& evt, CBasePlayer* m_pPlayer) {
	bool ducking = m_pPlayer->pev->flags & FL_DUCKING;

	if (ducking && (evt.punch.flags & FL_WC_PUNCH_STAND))
		return;
	if (!ducking && (evt.punch.flags & FL_WC_PUNCH_DUCK))
		return;

#ifdef CLIENT_DLL
	WC_EV_PunchAngle(evt, m_pPlayer->random_seed);
#else
	if (!m_pPlayer->IsSevenKewpClient()) {
		float punchAngleX = FP_10_6_TO_FLOAT(evt.punch.angles[0]);
		float punchAngleY = FP_10_6_TO_FLOAT(evt.punch.angles[1]);
		float punchAngleZ = FP_10_6_TO_FLOAT(evt.punch.angles[2]);

		if (!(evt.punch.flags & FL_WC_PUNCH_NO_RETURN)) {
			if (evt.punch.flags & FL_WC_PUNCH_ADD) {
				m_pPlayer->pev->punchangle = m_pPlayer->pev->punchangle + Vector(punchAngleX, punchAngleY, punchAngleZ);
			}
			else if (evt.punch.flags & FL_WC_PUNCH_SET) {
				m_pPlayer->pev->punchangle = Vector(punchAngleX, punchAngleY, punchAngleZ);
			}
			else {
				m_pPlayer->pev->punchangle = Vector(
					UTIL_SharedRandomFloat(m_pPlayer->random_seed, -punchAngleX, punchAngleX),
					UTIL_SharedRandomFloat(m_pPlayer->random_seed + 1, -punchAngleY, punchAngleY),
					UTIL_SharedRandomFloat(m_pPlayer->random_seed + 2, -punchAngleZ, punchAngleZ)
				);
			}
		}
	}
#endif
}

void CWeaponEvents::PlayEvent_WepAnim(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand) {
	if (!evt.anim.anims.arrSz)
		return;

	int idx = 0;

	if (evt.anim.flags & FL_WC_ANIM_ORDERED) {
		idx = (animCount++ % evt.anim.anims.arrSz);
	}
	else {
		idx = UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, evt.anim.anims.arrSz - 1);
	}

	int anim = evt.anim.anims.arr[idx];

	bool leftOnly = evt.anim.akimbo == WC_ANIM_LEFT_HAND || (evt.anim.akimbo == WC_ANIM_TRIG_HAND && leftHand);
	if (evt.anim.akimbo == WC_ANIM_BOTH_HANDS || leftOnly) {
		m_weapon->SendAkimboAnim(anim);
		if (leftOnly)
			return; // don't play the right hand event
	}

	if (evt.anim.flags & FL_WC_ANIM_PMODEL) {
		m_pPlayer->SetThirdPersonWeaponAnim(anim);
		return;
	}

#ifdef CLIENT_DLL
	cl_entity_t* vmodel = gEngfuncs.GetViewModel();
	int currentAnim = vmodel ? vmodel->curstate.sequence : 0;
	if ((evt.anim.flags & FL_WC_ANIM_NO_RESET) && anim == currentAnim) {
		return;
	}

	WC_EV_WepAnim(evt, m_weapon->m_iId, anim);
#else
	m_weapon->SendWeaponAnimSpec(anim);
#endif
}

void CWeaponEvents::PlayEvent_Cooldown(WepEvt& evt, CBasePlayer* m_pPlayer) {
	float nextAction = UTIL_WeaponTimeBase() + evt.cooldown.millis * 0.001f;
	if (evt.cooldown.targets & FL_WC_COOLDOWN_PRIMARY) {
		m_weapon->m_flNextPrimaryAttack = nextAction;
	}
	if (evt.cooldown.targets & FL_WC_COOLDOWN_SECONDARY) {
		m_weapon->m_flNextSecondaryAttack = nextAction;
	}
	if (evt.cooldown.targets & FL_WC_COOLDOWN_TERTIARY) {
		m_weapon->m_flNextTertiaryAttack = nextAction;
	}
	if (evt.cooldown.targets & FL_WC_COOLDOWN_IDLE) {
		m_weapon->m_flTimeWeaponIdle = nextAction;
	}
}

void CWeaponEvents::PlayEvent_ToggleState(WepEvt& evt, CBasePlayer* m_pPlayer) {
	bool toggleOn = evt.toggleState.toggleMode == WC_TOGGLE_STATE_ON;
	bool toggleFlip = evt.toggleState.toggleMode == WC_TOGGLE_STATE_TOGGLE;

	if (evt.toggleState.stateBits & FL_WC_STATE_IS_AKIMBO) {
		m_weapon->SetAkimbo(toggleOn || (toggleFlip && !m_weapon->IsAkimbo()));

		if (m_weapon->IsAkimbo()) {
			m_weapon->SendAkimboAnim(m_weapon->params.akimbo.deployAnim);
		}
		else {
			m_weapon->Deploy();
		}
	}

	if (evt.toggleState.stateBits & FL_WC_STATE_LASER) {
		m_weapon->ToggleLaser(toggleOn || (toggleFlip && !m_weapon->IsLaserOn()));
		ProcessEvents(m_weapon->IsLaserOn() ? WC_TRIG_LASER_ON : WC_TRIG_LASER_OFF, 0);
	}

	if (evt.toggleState.stateBits & FL_WC_STATE_PRIMARY_ALT) {
		m_weapon->SetPrimaryAlt(toggleOn || (toggleFlip && !m_weapon->IsPrimaryAltActive()));
	}
}

void CWeaponEvents::PlayEvent_HideLaser(WepEvt& evt, CBasePlayer* m_pPlayer) {
	if (m_weapon->IsLaserOn()) {
		m_weapon->HideLaser(true);
		m_weapon->m_laserOnTime = m_weapon->WallTime() + evt.laserHide.millis * 0.001f;
	}
}

void CWeaponEvents::PlayEvent_DLight(WepEvt& evt, CBasePlayer* m_pPlayer) {
#ifdef CLIENT_DLL
	WC_EV_Dlight(evt);
#else
	bool isPredicted = m_weapon->IsPredicted();

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr || (plr == m_pPlayer && isPredicted) || !m_pPlayer->InPVS(plr->edict()))
			continue;

		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, plr->edict());
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD_VECTOR(m_pPlayer->pev->origin);
		WRITE_BYTE(evt.dlight.radius);
		WRITE_BYTE(evt.dlight.color.r);
		WRITE_BYTE(evt.dlight.color.g);
		WRITE_BYTE(evt.dlight.color.b);
		WRITE_BYTE(evt.dlight.life);
		WRITE_BYTE(evt.dlight.decayRate);
		MESSAGE_END();
	}
#endif
}

void CWeaponEvents::PlayEvent_MuzzleFlash(WepEvt& evt, CBasePlayer* m_pPlayer) {
	switch (evt.muzzleFlash.brightness) {
	case WC_FLASH_DIM:
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
		QuakeMuzzleFlash(m_pPlayer);
		break;
	case WC_FLASH_NORMAL:
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
		QuakeMuzzleFlash(m_pPlayer);
		break;
	case WC_FLASH_BRIGHT:
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		QuakeMuzzleFlash(m_pPlayer);
		break;
	default:
		break;
	}
}

void CWeaponEvents::PlayEvent_SpriteTrail(WepEvt& evt, CBasePlayer* m_pPlayer, WcTrace* tr) {

	Vector start, end;

	if (tr) {
		start = tr->vecEndPos;
		end = start + tr->vecPlaneNormal;
	}
	else {
#ifdef CLIENT_DLL
		start = WC_GetGunPosition();
		end = start + WC_GetAim(0, 0);
#else
		start = m_pPlayer->GetGunPosition();
		end = start + m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
#endif		
	}

#ifdef CLIENT_DLL
	gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, start, end, evt.spriteTrail.sprite,
		evt.spriteTrail.count, 0.0, evt.spriteTrail.scale / 10.0f, evt.spriteTrail.speedNoise * 20,
		255, evt.spriteTrail.speed * 5);
#else
	bool isPredicted = m_weapon->IsPredicted();

	for (int i = 1; i < gpGlobals->time; i++) {
		CBasePlayer* listener = UTIL_PlayerByIndex(i);

		if (!listener) {
			continue;
		}

		if ((m_pPlayer != listener || !isPredicted) && UTIL_TestPVS(start, listener->edict())) {
			UTIL_SpriteTrail(start, end, evt.spriteTrail.sprite, evt.spriteTrail.count, 0,
				evt.spriteTrail.scale, evt.spriteTrail.speed, evt.spriteTrail.speedNoise,
				MSG_ONE_UNRELIABLE, NULL, listener->edict());
		}
	}

#endif
}

void CWeaponEvents::PlayEvent_Decal(WepEvt& evt, CBasePlayer* m_pPlayer, WcTrace* tr) {
	Vector pos;

	if (tr) {
		pos = tr->vecEndPos;
	}
	else {
#ifdef CLIENT_DLL
		pos = WC_GetGunPosition();
#else
		pos = m_pPlayer->GetGunPosition();
#endif		
	}

#ifdef CLIENT_DLL
	if (evt.decal.flags & FL_WC_DECAL_PARTICLES) {
		EV_HLDM_GunshotDecalEffects(pos, true);
	}

	gEngfuncs.pEfxAPI->R_DecalShoot(gEngfuncs.pEfxAPI->Draw_DecalIndex(evt.decal.decalIdx),
		tr ? tr->pHit : 0, 0, pos, 0);
#else
	bool isPredicted = m_weapon->IsPredicted();
	edict_t* ed = tr ? INDEXENT(tr->pHit) : NULL;

	for (int i = 1; i < gpGlobals->time; i++) {
		CBasePlayer* listener = UTIL_PlayerByIndex(i);

		if (!listener) {
			continue;
		}

		if ((m_pPlayer != listener || !isPredicted)) {
			if (evt.decal.flags & FL_WC_DECAL_PARTICLES) {
				UTIL_GunshotDecal(ed ? ENTINDEX(ed) : 0, pos, evt.decal.decalIdx, MSG_ONE_UNRELIABLE,
					NULL, listener->edict());
			}
			else {
				UTIL_Decal(ed ? ENTINDEX(ed) : 0, pos, evt.decal.decalIdx, MSG_ONE_UNRELIABLE,
					NULL, listener->edict());
			}
		}
	}

#endif
}

void CWeaponEvents::PlayEvent(int eventIdx, bool leftHand, bool akimboFire, WcTrace* tr) {
	CBasePlayer* m_pPlayer = m_weapon->GetPlayer();
	if (!m_pPlayer)
		return;

	Vector vecDir;
	WepEvt& evt = m_weapon->params.events[eventIdx];

	switch (evt.evtType) {
	case WC_EVT_SET_BODY:
		m_weapon->pev->body = evt.setBody.newBody;
		break;
	case WC_EVT_BULLETS:
		PlayEvent_Bullets(evt, m_pPlayer, leftHand, akimboFire);
		break;
	case WC_EVT_BEAM:
		PlayEvent_Beam(evt, m_pPlayer);
		break;
	case WC_EVT_PROJECTILE:
		PlayEvent_Projectile(evt, m_pPlayer);
		break;
	case WC_EVT_KICKBACK:
		PlayEvent_Kickback(evt, m_pPlayer);
		break;
	case WC_EVT_EJECT_SHELL:
		PlayEvent_EjectShell(evt, m_pPlayer, leftHand);
		break;
	case WC_EVT_PUNCH:
		PlayEvent_PunchAngle(evt, m_pPlayer);
		break;
	case WC_EVT_IDLE_SOUND:
		PlayEvent_Sound(evt, m_pPlayer, leftHand, akimboFire);
		break;
	case WC_EVT_PLAY_SOUND:
		PlayEvent_Sound(evt, m_pPlayer, leftHand, akimboFire);
		if (evt.playSound.flags & FL_WC_SOUND_CHARGE_PITCH) {
			m_weapon->m_chargeSoundEvt = eventIdx;
		}
		break;
	case WC_EVT_TOGGLE_ZOOM:
		m_weapon->ToggleZoom(evt.zoomToggle.zoomFov, evt.zoomToggle.zoomFov2);
		ProcessEvents(m_pPlayer->m_iFOV ? WC_TRIG_ZOOM_IN : WC_TRIG_ZOOM_OUT, 0);
		break;
	case WC_EVT_TOGGLE_STATE:
		PlayEvent_ToggleState(evt, m_pPlayer);
		break;
	case WC_EVT_HIDE_LASER:
		PlayEvent_HideLaser(evt, m_pPlayer);
		break;
	case WC_EVT_WEP_ANIM:
		PlayEvent_WepAnim(evt, m_pPlayer, leftHand);
		break;
	case WC_EVT_COOLDOWN:
		PlayEvent_Cooldown(evt, m_pPlayer);
		break;
	case WC_EVT_SET_GRAVITY:
		PlayEvent_SetGravity(evt, m_pPlayer);
		break;
	case WC_EVT_DLIGHT:
		PlayEvent_DLight(evt, m_pPlayer);
		break;
	case WC_EVT_MUZZLEFLASH:
		PlayEvent_MuzzleFlash(evt, m_pPlayer);
		break;
	case WC_EVT_SPRITETRAIL:
		PlayEvent_SpriteTrail(evt, m_pPlayer, tr);
		break;
	case WC_EVT_DECAL:
		PlayEvent_Decal(evt, m_pPlayer, tr);
		break;
	case WC_EVT_SERVER:
		m_weapon->CustomServerEvent(evt, m_pPlayer);
		break;
	default:
		ALERT(at_error, "Unhandled weapon event type %d\n", evt.evtType);
		break;
	}
}

void CWeaponEvents::PlayDelayedEvents() {
#ifdef CLIENT_DLL
	if (!g_runfuncs)
		return;
#endif

	for (int i = 0; i < WC_SERVER_EVENT_QUEUE_SZ; i++) {
		WcDelayEvent& qevt = eventQueue[i];

		if (!qevt.fireTime || qevt.fireTime > m_weapon->WallTime())
			continue;

		PlayEvent(qevt.eventIdx, qevt.leftHand, qevt.akimboFire, &qevt.tr);
		qevt.fireTime = 0; // free the slot
	}
}

void CWeaponEvents::CancelDelayedEvents(int trigger) {
	if (!g_runfuncs)
		return;

	for (int i = 0; i < WC_SERVER_EVENT_QUEUE_SZ; i++) {
		WcDelayEvent& qevt = eventQueue[i];
		WepEvt& evt = m_weapon->params.events[qevt.eventIdx];

		if (evt.trigger == trigger) {
			qevt.fireTime = 0;
		}
	}
}


float CWeaponEvents::GetCurrentAccuracyMultiplier() {
	CBasePlayer* m_pPlayer = m_weapon->GetPlayer();
	if (!m_pPlayer)
		return 1.0f;

	float multiplier = 1.0f;

	Vector flatVelocity = m_pPlayer->pev->velocity;
	flatVelocity.z = 0;

	bool isMoving = flatVelocity.Length() > 200;

	if (!(m_pPlayer->pev->flags & FL_ONGROUND)) {
		multiplier *= 3.0f;
	}
	else if (m_pPlayer->pev->flags & FL_DUCKING) {
		multiplier *= 0.5f;
	}
	else if (isMoving) {
		multiplier *= 2.0f;
	}

	return multiplier;
}

void CWeaponEvents::GetCurrentAccuracy(float& accuracyX, float& accuracyY, float& accuracyX2, float& accuracyY2) {
	CBasePlayer* m_pPlayer = m_weapon->GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponParams& params = m_weapon->params;

	if (m_weapon->IsPrimaryAltActive()) {
		accuracyX = params.shootOpts[3].accuracy[0] * 0.01f;
		accuracyY = params.shootOpts[3].accuracy[1] * 0.01f;
	}
	else if (m_weapon->IsAkimbo()) {
		accuracyX = params.akimbo.accuracy[0] * 0.01f;
		accuracyY = params.akimbo.accuracy[1] * 0.01f;
	}
	else {
		accuracyX = params.shootOpts[0].accuracy[0] * 0.01f;
		accuracyY = params.shootOpts[0].accuracy[1] * 0.01f;
	}

	bool hasSecondary = params.flags & FL_WC_WEP_HAS_SECONDARY;
	bool secondaryShoots = !(params.shootOpts[1].flags & FL_WC_SHOOT_NO_ATTACK);

	if (hasSecondary && secondaryShoots) {
		accuracyX2 = params.shootOpts[1].accuracy[0] * 0.01f;
		accuracyY2 = params.shootOpts[1].accuracy[1] * 0.01f;
	}
	else {
		accuracyX2 = accuracyX;
		accuracyY2 = accuracyY;
	}

	if (params.flags & FL_WC_WEP_DYNAMIC_ACCURACY) {
		float multiplier = GetCurrentAccuracyMultiplier();

		accuracyX *= multiplier;
		accuracyY *= multiplier;
		accuracyX2 *= multiplier;
		accuracyY2 *= multiplier;
	}
}

int CWeaponEvents::GetImpactArg(int attackIdx, bool impactMonster, bool impactWorld) {
	switch (attackIdx) {
	default:
	case 0:
		if (impactMonster && impactWorld)	return WC_TRIG_IMPACT_PRIMARY_ANY;
		if (impactMonster)					return WC_TRIG_IMPACT_PRIMARY_MONSTER;
		return WC_TRIG_IMPACT_PRIMARY_WORLD;
	case 1:
		if (impactMonster && impactWorld)	return WC_TRIG_IMPACT_SECONDARY_ANY;
		if (impactMonster)					return WC_TRIG_IMPACT_SECONDARY_MONSTER;
		return WC_TRIG_IMPACT_SECONDARY_WORLD;
	case 2:
		if (impactMonster && impactWorld)	return WC_TRIG_IMPACT_TERTIARY_ANY;
		if (impactMonster)					return WC_TRIG_IMPACT_TERTIARY_MONSTER;
		return WC_TRIG_IMPACT_TERTIARY_WORLD;
	case 3:
		if (impactMonster && impactWorld)	return WC_TRIG_IMPACT_PRIMARY_ALT_ANY;
		if (impactMonster)					return WC_TRIG_IMPACT_PRIMARY_ALT_MONSTER;
		return WC_TRIG_IMPACT_PRIMARY_ALT_WORLD;
	}
}

float CWeaponEvents::GetChargeMult(WepEvt& evt, int flagMask) {
	int attackIdx = m_weapon->GetAttackIdx(evt);
	if (m_weapon->params.flags & FL_WC_WEP_LINK_CHARGEUPS) {
		attackIdx = 0;
	}
	CustomWeaponShootOpts& opts = m_weapon->GetShootOpts(attackIdx);

	if (!opts.chargeTime)
		return 1.0f;

	if (!(opts.chargeFlags & flagMask))
		return 1.0f;

	uint32_t chargeMillis = m_weapon->CmdTime() - m_weapon->m_chargeStartCmdTime;
	return V_min(chargeMillis / (float)opts.chargeTime, 1.0f);
}

Vector CWeaponEvents::BeamAttack(WcBeam& beam, CBasePlayer* m_pPlayer) {
	Vector spread(SPREAD_TO_FLOAT(beam.evt.beam.accuracy[0]), SPREAD_TO_FLOAT(beam.evt.beam.accuracy[1]), 0);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	bool isPredicted = m_weapon->IsPredicted();
	BULLET_PREDICTION predFlag = isPredicted ? BULLETPRED_EVENTLESS : BULLETPRED_NONE;

	float damage = beam.evt.beam.damage * GetChargeMult(beam.evt, FL_WC_CHARGE_DAMAGE);

	lagcomp_begin(m_pPlayer);
	Vector vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, spread, beam.evt.beam.distance,
		BULLET_BEAM, 0, damage, m_pPlayer->pev, m_pPlayer->random_seed, g_traces, predFlag);
	lagcomp_end();

	beam.spreadX = vecDir.x;
	beam.spreadY = vecDir.y;

#ifdef CLIENT_DLL
	vecSrc = WC_GetGunPosition();
	Vector vecEnd = vecSrc + WC_GetAim(beam.spreadX, beam.spreadY) * beam.evt.beam.distance;
	pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);
	Vector endPos = tr.endpos;
	WcTrace evTrace = ConvertTrace(tr);
#else
	Vector endPos = g_traces[0].vecEndPos;
	WcTrace evTrace = ConvertTrace(g_traces[0]);
#endif

	int attackIdx = m_weapon->GetAttackIdx(beam.evt);
	ProcessEvents(WC_TRIG_IMPACT, GetImpactArg(attackIdx, true, true), false, false, 0, &evTrace);
	m_weapon->AttackTrace(m_pPlayer, attackIdx, vecSrc, g_traces[0]);

	// make splashes if this isn't an infinite beam
	if (beam.evt.beam.life != 0) {
		float splashSize = 0.3f;
		if (damage > 50) {
			splashSize = 0.5f;
		}
		else if (damage > 8) {
			splashSize = 0.4f;
		}
		edict_t* skipEnt = m_pPlayer->IsSevenKewpClient() ? m_pPlayer->edict() : NULL;
		UTIL_WaterSplashTrace(vecSrc, endPos, splashSize, 2, skipEnt);
	}

	beam.attackIdx = beam.evt.beam.life ? -1 : attackIdx;
	beam.nextAttack = !beam.evt.beam.life ? m_weapon->WallTime() + beam.evt.beam.freq * 0.001f : 0;

	return endPos;
}

void CWeaponEvents::FireAmmoEvents(int ammoPool) {
	CBasePlayer* m_pPlayer = m_weapon->GetPlayer();
	if (!m_pPlayer)
		return;
	if (!g_runfuncs)
		return;

	switch (ammoPool) {
	case WC_AMMOPOOL_PRIMARY_CLIP:
		ProcessEvents(WC_TRIG_PRIMARY_CLIP_SP, m_bulletFireCount++, m_weapon->IsAkimbo(), false, m_weapon->m_iClip);
		ProcessEvents(WC_TRIG_PRIMARY_CLIPSIZE, m_weapon->m_iClip, m_weapon->IsAkimbo(), false, m_weapon->m_iClip);
		break;
	case WC_AMMOPOOL_PRIMARY_RESERVE: break;
	case WC_AMMOPOOL_SECONDARY_RESERVE: break;
	default: break;
	}
}

WcBeam* CWeaponEvents::AllocBeam() {
	for (int i = 0; i < MAX_WC_BEAMS; i++) {
		if (m_beams[i].isFree()) {
			memset(&m_beams[i], 0, sizeof(WcBeam));
			return &m_beams[i];
		}
	}

	ALERT(at_warning, "Exceeded max weapon custom beams (%d)\n", MAX_WC_BEAMS);
	return NULL;
}

void CWeaponEvents::UpdateBeams() {
	CBasePlayer* m_pPlayer = m_weapon->GetPlayer();
	if (!m_pPlayer)
		return;

#ifdef CLIENT_DLL
	Vector vecSrc = WC_GetGunPosition();
#else
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
#endif

	for (int i = 0; i < MAX_WC_BEAMS; i++) {
		if (m_beams[i].isFree()) {
			continue;
		}
		WcBeam& beam = m_beams[i];
		bool isConstantBeam = beam.evt.beam.life == 0;

		if (beam.nextAttack && beam.nextAttack < m_weapon->WallTime()) {
			Vector attackPos = BeamAttack(beam, m_pPlayer);

			if (!isConstantBeam) {
#ifdef CLIENT_DLL
				beam.pBeam->target = attackPos;
#else
				CBeam* pbeam = (CBeam*)beam.h_beam.GetEntity();
				pbeam->SetStartPos(attackPos);
#endif
			}
		}

		if (beam.evt.beam.altMode != WC_BEAM_ANIM_DISABLED && beam.evt.beam.altTime) {
			float dur = beam.evt.beam.altTime * 0.001f;
			float p = normalizeRangef(gpGlobals->time - beam.creationTime, 0, dur) / dur; // progress
			float q = 1.0f - p; // progress left

			int t = p * 10000; // convert to int for modulo op later
			int freq = dur * 10000;  // time to alternate (half a cycle)

			switch (beam.evt.beam.altMode)
			{
			case WC_BEAM_ANIM_LINEAR:
			case WC_BEAM_ANIM_LINEAR_TOGGLE: p = p; break;
			case WC_BEAM_ANIM_TOGGLE: p = (p < 0.5) ? 0 : 1; break;
			case WC_BEAM_ANIM_EASE_IN_OUT:   p = p * p * p / (p * p * p + q * q * q); break;
			}

			if (beam.evt.beam.altMode != WC_BEAM_ANIM_TOGGLE && t % (freq * 2) >= freq)
				p = 1.0f - p;

			RGBA C;
			int width, noise, scroll;

			{	// color interp
				RGBA A = beam.evt.beam.color;
				RGBA B = beam.evt.beam.colorAlt;
				int dr = ((int)B.r - (int)A.r) * p + 0.5f;
				int dg = ((int)B.g - (int)A.g) * p + 0.5f;
				int db = ((int)B.b - (int)A.b) * p + 0.5f;
				int da = ((int)B.a - (int)A.a) * p + 0.5f;
				C = RGBA(A.r + dr, A.g + dg, A.b + db, A.a + da);
			}
			{	// width interp
				int a = beam.evt.beam.width;
				int b = beam.evt.beam.widthAlt;
				int d = (b - a) * p + 0.5f;
				width = a + d;
			}
			{	// noise interp
				int a = beam.evt.beam.noise;
				int b = beam.evt.beam.noiseAlt;
				int d = (b - a) * p + 0.5f;
				noise = a + d;
			}
			{	// scroll interp
				int a = beam.evt.beam.scrollRate;
				int b = beam.evt.beam.scrollRateAlt;
				int d = (b - a) * p + 0.5f;
				scroll = a + d;
			}

#ifdef CLIENT_DLL
			beam.pBeam->r = C.r / 255.0f;
			beam.pBeam->g = C.g / 255.0f;
			beam.pBeam->b = C.b / 255.0f;
			beam.pBeam->brightness = C.a / 255.0f;
			beam.pBeam->width = width / 10.0f;
			beam.pBeam->amplitude = noise / 100.0f;
			beam.pBeam->speed = scroll;
#else
			CBeam* pbeam = (CBeam*)beam.h_beam.GetEntity();
			if (pbeam) {
				pbeam->SetColor(C.r, C.g, C.b);
				pbeam->SetBrightness(C.a);
				pbeam->SetWidth(width);
				pbeam->SetNoise(noise);
				pbeam->SetScrollRate(scroll);
			}
#endif
		}

		if (!isConstantBeam)
			continue;

#ifdef CLIENT_DLL
		Vector vecDir = WC_GetAim(beam.spreadX, beam.spreadY);
		Vector vecEnd = vecSrc + vecDir * beam.evt.beam.distance;

		pmtrace_t tr;
		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

		beam.pBeam->target = tr.endpos;

		if (m_beamImpactSprite.pSprite) {
			if (i == m_beamImpactSprite.beamId) {
				m_beamImpactSprite.pSprite->entity.origin = tr.endpos;
			}
			if (m_beamImpactSprite.killTime && m_beamImpactSprite.killTime < gpGlobals->time) {
				m_beamImpactSprite.Kill();
			}
		}
#else
		Vector vecDir = vecAiming + beam.spreadX * gpGlobals->v_right + beam.spreadY * gpGlobals->v_up;
		Vector vecEnd = vecSrc + vecDir * beam.evt.beam.distance;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_weapon->pev), &tr);

		CBeam* pbeam = (CBeam*)beam.h_beam.GetEntity();
		if (pbeam) {
			pbeam->SetStartPos(tr.vecEndPos);
			if (m_beamImpactSprite.h_sprite) {
				if (i == m_beamImpactSprite.beamId) {
					UTIL_SetOrigin(m_beamImpactSprite.h_sprite->pev, tr.vecEndPos);
				}
				if (m_beamImpactSprite.killTime && m_beamImpactSprite.killTime < gpGlobals->time) {
					m_beamImpactSprite.Kill();
				}
			}
		}
#endif
	}
}

bool CWeaponEvents::KillBeams(int attackIdx) {
	bool anyKilled = false;

	for (int i = 0; i < MAX_WC_BEAMS; i++) {
		if (m_beams[i].isFree())
			continue;

		if (attackIdx == -1 || (attackIdx == m_beams[i].attackIdx)) {
#ifdef CLIENT_DLL
			m_beams[i].pBeam->die = 0.0f;
#else
			UTIL_Remove(m_beams[i].h_beam);
#endif
			memset(&m_beams[i], 0, sizeof(WcBeam));
			anyKilled = true;

			if (m_beamImpactSprite.IsAlive() && i == m_beamImpactSprite.beamId) {
				m_beamImpactSprite.Kill();
			}
		}
	}

	if (attackIdx == -1) {
		m_beamImpactSprite.Kill();
	}

	return anyKilled;
}

bool CWeaponEvents::CheckTracer(int idx, Vector& vecSrc, Vector forward, Vector right, int iTracerFreq)
{
	if (idx < 0 || idx >= 32) {
		return false;
	}

	if (iTracerFreq != 0 && ((m_tracerCount[idx])++ % iTracerFreq) == 0)
	{
		// adjust for player
		vecSrc = vecSrc + Vector(0, 0, -4) + right * 2 + forward * 16;
		return true;
	}

	return false;
}

void CWeaponEvents::QuakeMuzzleFlash(CBasePlayer* plr) {
#ifdef CLIENT_DLL
	EV_MuzzleFlash();
#else
	plr->pev->effects |= EF_MUZZLEFLASH;
#endif
}