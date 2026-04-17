
enum MovementType {
	PWPT_WALK,
	PWPT_RUN,
	PWPT_TELEPORT,
	PWPT_TURN_TO_FACE,
	PWPT_DONT_MOVE,
};

enum PathWaypointWaitActivity {
	PWPT_WAIT_PLAY_ANIM,
	PWPT_LOOK_AROUND,	// wildly change YAW
	PWPT_INVESTIGATE,	// unimplemented, same as play anim
	PWPT_USE_AI,		// unimplemented, same as play anim
};

enum WaypointState {
	PWPT_STATE_ARRIVE,
	PWPT_STATE_FACE_IDEAL,
	PWPT_STATE_WAIT,
	PWPT_STATE_DEPART,
	PWPT_STATE_NEXT_WAYPOINT
};

// a mostly unimplemented entity. In sven it seems to behave like a chain of scripted_sequence
// except that you can kill the monster, and there are more events per sequence. Monsters also
// cancel their route if the waypoint they're heading for is full (number of nearby monsters > max occupants).

class CPathWaypoint : public CCineMonster
{
public:
	void Spawn();
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	CPathWaypoint* MyPathWaypointPointer(void) { return this; }

	Schedule_t* GetScriptSchedule() override;
	void DoScript(CBaseMonster* pTarget) override;

	void FinishRoute(CBaseMonster* pMonster);
	bool IsFull(CBaseMonster* checker);

	bool m_useAngles;	// turn to face waypoint angles on arrival
	string_t m_alternateTarget;

	string_t m_triggerOnArrival;
	string_t m_triggerAfterArrivalAnim;
	string_t m_arrivalAnim;

	int m_waitActivity; // PathWaypointWaitActivity
	string_t m_waitAnim;
	float m_waitTime;
	string_t m_waitMaster;	// wait until master is activated
	//bool m_waitUntilFull; // UNIMPLEMENTED: Force monsters to wait until this waypoint is full before proceeding to the next waypoint. If Maximum Occupants is 0, a level error message will occur.

	string_t m_departureAnim;
	string_t m_triggerOnDeparture;

	float m_occupantRadius;	// any monster within this radius is occupies the waypoint
	int m_occupantLimit;	// prevents multiple monsters heading for the same waypoint. In that case, the blocked monsters cancel their routes
	//string_t m_overflowWaypoint; // UNIMPLEMENTED: If this waypoint is full, monsters will go to the specified waypoint instead.If the provided overflow point is removed or not available, monster will remain at the previous waypoint or position until room is available.

	//bool m_forceCompletion; // UNIMPLEMENTED: Forces monster to finish this particular path_waypoint even if a higher priority path is found, provided that the stop conditions are not met. This disables path_condition / path_condition_controller evaluation during the extent of the trip and departure.
	//string_t m_triggerOnStop; // UNIMPLEMENTED: If this path_waypoint releases monster because of a stop condition, (e.g. a new enemy appears) something can be triggered.
	//float m_restartDelay; // UNIMPLEMENTED: If monster has stopped moving because of a stop condition (E.g. hearing a gunshot), the NPC will wait this long before continuing back to this path_waypoint.

	float m_nextLookAround;
};