#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "customentity.h"
#include "effects.h"
#include "weapons.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"

class CGlow : public CPointEntity
{
public:
	void Spawn(void);
	void Think(void);
	void Animate(float frames);
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	float		m_lastTime;
	float		m_maxFrame;
};

LINK_ENTITY_TO_CLASS(env_glow, CGlow);

TYPEDESCRIPTION	CGlow::m_SaveData[] =
{
	DEFINE_FIELD(CGlow, m_lastTime, FIELD_TIME),
	DEFINE_FIELD(CGlow, m_maxFrame, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CGlow, CPointEntity);

void CGlow::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	PRECACHE_MODEL((char*)STRING(pev->model));
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
	if (m_maxFrame > 1.0 && pev->framerate != 0)
		pev->nextthink = gpGlobals->time + 0.1;

	m_lastTime = gpGlobals->time;
}


void CGlow::Think(void)
{
	Animate(pev->framerate * (gpGlobals->time - m_lastTime));

	pev->nextthink = gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}


void CGlow::Animate(float frames)
{
	if (m_maxFrame > 0)
		pev->frame = fmod(pev->frame + frames, m_maxFrame);
}
