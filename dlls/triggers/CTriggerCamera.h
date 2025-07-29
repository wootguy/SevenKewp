#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"

#define SF_CAMERA_PLAYER_POSITION	1 // camera position starts at the activator
#define SF_CAMERA_PLAYER_TARGET		2 // camera follows the activator
#define SF_CAMERA_PLAYER_TAKECONTROL 4 // freeze viewers
#define SF_CAMERA_ALL_PLAYERS 8 // force everyone to view, dead or alive
#define SF_CAMERA_FORCE_VIEW 16 // activator is forced to view even if dead ("all players" also does this)
#define SF_CAMERA_INSTANT_TURN 32 // if enabled, don't smoothly rotate to face target
#define SF_CAMERA_PLAYER_INVULNERABLE 256 // disable viewer damage while camera is active

class EXPORT CTriggerCamera : public CBaseDelay
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void FollowTarget(void);
	void Move(void);
	void TogglePlayerViews(bool enabled);
	void TogglePlayerView(CBasePlayer* plr, bool enabled);
	virtual CTriggerCamera* MyCameraPointer(void) { return this; }
	virtual int AddToFullPack(struct entity_state_s* state, CBasePlayer* player);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;
	CBaseEntity* m_pentPath;
	int	  m_sPath;
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	float m_initialSpeed;
	float m_acceleration;
	float m_deceleration;
	float m_turnspeed;
	int	  m_state;
	float m_activateTime; // time camera was activated
	string_t m_iszTurnedOffTarget;
};

// currently active "All players" camera. Sent to new joiners.
extern EXPORT EHANDLE g_active_camera;