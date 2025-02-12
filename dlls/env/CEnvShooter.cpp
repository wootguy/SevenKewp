#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "customentity.h"
#include "effects.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"
#include "CGibShooter.h"

#define SF_ENVSHOOTER_DONT_WAIT_TILL_LAND 4

class CEnvShooter : public CGibShooter
{
	void		Precache(void);
	void		KeyValue(KeyValueData* pkvd);

	CGib *CreateGib( float lifeTime );
};

LINK_ENTITY_TO_CLASS(env_shooter, CEnvShooter)

void CEnvShooter::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "shootmodel"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shootsounds"))
	{
		int iNoise = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
		switch (iNoise)
		{
		case 0:
			m_iGibMaterial = matGlass;
			break;
		case 1:
			m_iGibMaterial = matWood;
			break;
		case 2:
			m_iGibMaterial = matMetal;
			break;
		case 3:
			m_iGibMaterial = matFlesh;
			break;
		case 4:
			m_iGibMaterial = matRocks;
			break;

		default:
		case -1:
			m_iGibMaterial = matNone;
			break;
		}
	}
	else
	{
		CGibShooter::KeyValue(pkvd);
	}
}


void CEnvShooter::Precache(void)
{
	m_iGibModelIndex = PRECACHE_MODEL((char*)STRING(pev->model));
	CBreakable::MaterialSoundPrecache((Materials)m_iGibMaterial);
}


CGib *CEnvShooter::CreateGib( float lifeTime )
{
	CGib* pGib = GetClassPtr((CGib*)NULL);
	if (!pGib)
		return NULL;

	pGib->Spawn(STRING(pev->model));
	pGib->m_lifeTime = lifeTime;

	if (FBitSet(pev->spawnflags, SF_ENVSHOOTER_DONT_WAIT_TILL_LAND))
	{
		pGib->SetThink( &CGib::StartFadeOut );
		pGib->pev->nextthink = gpGlobals->time + lifeTime;
	}

	int bodyPart = 0;

	if (pev->body > 1)
		bodyPart = RANDOM_LONG(0, pev->body - 1);

	pGib->pev->body = bodyPart;
	pGib->m_bloodColor = DONT_BLEED;
	pGib->m_material = m_iGibMaterial;

	pGib->pev->rendermode = pev->rendermode;
	pGib->pev->renderamt = pev->renderamt;
	pGib->pev->rendercolor = pev->rendercolor;
	pGib->pev->renderfx = pev->renderfx;
	pGib->pev->scale = pev->scale;
	pGib->pev->skin = pev->skin;

	return pGib;
}

