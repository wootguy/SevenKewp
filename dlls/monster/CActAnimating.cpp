#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "animation.h"
#include "effects.h"
#include "CActAnimating.h"

TYPEDESCRIPTION	CActAnimating::m_SaveData[] =
{
	DEFINE_FIELD(CActAnimating, m_Activity, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CActAnimating, CBaseAnimating);

void CActAnimating::SetActivity(Activity act)
{
	int sequence = LookupActivity(act);
	if (sequence != ACTIVITY_NOT_AVAILABLE)
	{
		pev->sequence = sequence;
		m_Activity = act;
		pev->frame = 0;
		ResetSequenceInfo();
	}
}
