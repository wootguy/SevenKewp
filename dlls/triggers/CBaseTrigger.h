#pragma once

#define SF_TRIGGER_HURT_TARGETONCE	1// Only fire hurt target once
#define	SF_TRIGGER_HURT_START_OFF	2//spawnflag that makes trigger_push spawn turned OFF
#define	SF_TRIGGER_HURT_NO_CLIENTS	8//spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_CLIENTONLYFIRE	16// trigger hurt will only fire its target if it is hurting a client
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32// only clients may touch this trigger.

class CBaseTrigger : public CBaseToggle
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void KeyValue(KeyValueData* pkvd);
	void EXPORT MultiTouch(CBaseEntity* pOther);
	void EXPORT HurtTouch(CBaseEntity* pOther);
	void EXPORT CDAudioTouch(CBaseEntity* pOther);
	void ActivateMultiTrigger(CBaseEntity* pActivator);
	void EXPORT MultiWaitOver(void);
	void EXPORT CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void InitTrigger(void);

	virtual int	ObjectCaps(void) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};