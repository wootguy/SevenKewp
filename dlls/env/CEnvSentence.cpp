#include "extdll.h"
#include "util.h"
#include "cbase.h"

#define SF_EMSG_ACTIVATOR_ONLY 1

class CEnvSentence : public CPointEntity
{
public:
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	KeyValue(KeyValueData* pkvd);

	string_t message;
private:
};

LINK_ENTITY_TO_CLASS(env_sentence, CEnvSentence)

void CEnvSentence::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "_text"))
	{
		message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}


void CEnvSentence::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (message) {
		const char* spkString = UTIL_VarArgs(";spk \"%s\";", STRING(message));

		if (pev->spawnflags & SF_EMSG_ACTIVATOR_ONLY) {
			if (pActivator->IsPlayer()) {
				MESSAGE_BEGIN(MSG_ONE, SVC_STUFFTEXT, NULL, pActivator->edict());
				WRITE_STRING(spkString);
				MESSAGE_END();
			}
		}
		else
		{
			MESSAGE_BEGIN(MSG_ALL, SVC_STUFFTEXT);
			WRITE_STRING(spkString);
			MESSAGE_END();
		}
	}

	SUB_UseTargets(this, USE_TOGGLE, 0);
}
