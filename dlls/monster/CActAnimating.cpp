#include "extdll.h"
#include "util.h"
#include "animation.h"
#include "effects.h"
#include "CActAnimating.h"

TYPEDESCRIPTION	CActAnimating::m_SaveData[] =
{
	DEFINE_FIELD(CActAnimating, m_Activity, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CActAnimating, CBaseAnimating)

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

void CActAnimating::DropToFloor()
{
	if( DROP_TO_FLOOR(ENT( pev ) ) == 0 )
	{
		ALERT(at_error, "Entity %s fell out of level at %f,%f,%f\n", STRING( pev->classname ), pev->origin.x, pev->origin.y, pev->origin.z);
		UTIL_Remove( this );
	}
}