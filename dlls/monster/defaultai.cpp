/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// Default behaviors.
//=========================================================
#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"defaultai.h"
#include	"env/CSoundEnt.h"
#include	"nodes.h"
#include	"scripted.h"

//=========================================================
// Fail
//=========================================================
Task_t	tlFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slFail[] =
{
	{
		tlFail,
		ARRAYSIZE ( tlFail ),
		bits_COND_CAN_ATTACK |

		bits_COND_NEW_ENEMY |
		bits_COND_SEE_ENEMY |
		bits_COND_SEE_FEAR |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_PROVOKED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT |// sound flags
		bits_SOUND_WORLD |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER,
		"FAIL"
	},
};

//=========================================================
//	Idle Schedules
//=========================================================
Task_t	tlIdleStand1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)5		},// repick IDLESTAND every five seconds. gives us a chance to pick an active idle, fidget, etc.
};

Schedule_t	slIdleStand[] =
{
	{ 
		tlIdleStand1,
		ARRAYSIZE ( tlIdleStand1 ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL_FOOD	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER		|

		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"IDLE_STAND"
	},
};

Schedule_t	slIdleTrigger[] =
{
	{ 
		tlIdleStand1,
		ARRAYSIZE ( tlIdleStand1 ), 
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"IDLE_TRIGGER"
	},
};


Task_t	tlIdleWalk1[] =
{
	{ TASK_WALK_PATH,			(float)9999 },
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0	},
};

Schedule_t	slIdleWalk[] =
{
	{ 
		tlIdleWalk1,
		ARRAYSIZE ( tlIdleWalk1 ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL_FOOD	|
		bits_COND_SMELL			|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags

		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"IDLE_WALK"
	},
};

//=========================================================
// Ambush - monster stands in place and waits for a new 
// enemy, or chance to attack an existing enemy.
//=========================================================
Task_t	tlAmbush[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_INDEFINITE,		(float)0		},
};

Schedule_t	slAmbush[] =
{
	{ 
		tlAmbush,
		ARRAYSIZE ( tlAmbush ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_PROVOKED,

		0,
		"AMBUSH"
	},
};

//=========================================================
// ActiveIdle schedule - !!!BUGBUG - if this schedule doesn't
// complete on its own, the monster's HintNode will not be 
// cleared, and the rest of the monster's group will avoid
// that node because they think the group member that was 
// previously interrupted is still using that node to active
// idle.
///=========================================================
Task_t tlActiveIdle[] =
{
	{ TASK_FIND_HINTNODE,			(float)0	},
	{ TASK_GET_PATH_TO_HINTNODE,	(float)0	},
	{ TASK_STORE_LASTPOSITION,		(float)0	},
	{ TASK_WALK_PATH,				(float)0	},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0	},
	{ TASK_FACE_HINTNODE,			(float)0	},
	{ TASK_PLAY_ACTIVE_IDLE,		(float)0	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0	},
	{ TASK_WALK_PATH,				(float)0	},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0	},
	{ TASK_CLEAR_LASTPOSITION,		(float)0	},
	{ TASK_CLEAR_HINTNODE,			(float)0	},
};

Schedule_t slActiveIdle[] =
{
	{
		tlActiveIdle,
		ARRAYSIZE( tlActiveIdle ),
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_PROVOKED		|
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT		|
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER,
		"ACTIVE_IDLE"
	}
};

//=========================================================
//	Wake Schedules
//=========================================================
Task_t tlWakeAngry1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SOUND_WAKE,			(float)0	},
	{ TASK_FACE_IDEAL,			(float)0	},
};

Schedule_t slWakeAngry[] =
{
	{
		tlWakeAngry1,
		ARRAYSIZE ( tlWakeAngry1 ),
		bits_COND_NEW_ENEMY,
		0,
		"WAKE_ANGRY"
	}
};

//=========================================================
// AlertFace Schedules
//=========================================================
Task_t	tlAlertFace1[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	{ TASK_FACE_IDEAL,				(float)0		},
};

Schedule_t	slAlertFace[] =
{
	{ 
		tlAlertFace1,
		ARRAYSIZE ( tlAlertFace1 ),
		bits_COND_NEW_ENEMY		|
		bits_COND_SEE_FEAR		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_PROVOKED,
		0,
		"ALERT_FACE"
	},
};

//=========================================================
// AlertSmallFlinch Schedule - shot, but didn't see attacker,
// flinch then face
//=========================================================
Task_t	tlAlertSmallFlinch[] =
{
	{ TASK_STOP_MOVING,				0						},
	{ TASK_REMEMBER,				(float)bits_MEMORY_FLINCHED },
	{ TASK_SMALL_FLINCH,			(float)0				},
	{ TASK_SET_SCHEDULE,			(float)SCHED_ALERT_FACE	},
};

Schedule_t	slAlertSmallFlinch[] =
{
	{ 
		tlAlertSmallFlinch,
		ARRAYSIZE ( tlAlertSmallFlinch ),
		0,
		0,
		"ALERT_SMALL_FLINCH"
	},
};

//=========================================================
// AlertIdle Schedules
//=========================================================
Task_t	tlAlertStand1[] =
{
	{ TASK_STOP_MOVING,			0						 },
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE			 },
	{ TASK_WAIT,				(float)20				 },
	{ TASK_SUGGEST_STATE,		(float)MONSTERSTATE_IDLE },
};

Schedule_t	slAlertStand[] =
{
	{ 
		tlAlertStand1,
		ARRAYSIZE ( tlAlertStand1 ), 
		bits_COND_NEW_ENEMY				|
		bits_COND_SEE_ENEMY				|
		bits_COND_SEE_FEAR				|
		bits_COND_LIGHT_DAMAGE			|
		bits_COND_HEAVY_DAMAGE			|
		bits_COND_PROVOKED				|
		bits_COND_SMELL					|
		bits_COND_SMELL_FOOD			|
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT		|// sound flags
		bits_SOUND_WORLD		|
		bits_SOUND_PLAYER		|
		bits_SOUND_DANGER		|

		bits_SOUND_MEAT			|// scent flags
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"ALERT_SOUND"
	},
};

//=========================================================
// InvestigateSound - sends a monster to the location of the
// sound that was just heard, to check things out. 
//=========================================================
Task_t tlInvestigateSound[] =
{
	{ TASK_STOP_MOVING,				(float)0				},
	{ TASK_STORE_LASTPOSITION,		(float)0				},
	{ TASK_GET_PATH_TO_BESTSOUND,	(float)0				},
	{ TASK_FACE_IDEAL,				(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_IDLE			},
	{ TASK_WAIT,					(float)10				},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_WALK_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slInvestigateSound[] =
{
	{ 
		tlInvestigateSound,
		ARRAYSIZE ( tlInvestigateSound ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_SEE_FEAR			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"INVESTIGATE_SOUND"
	},
};

//=========================================================
// CombatIdle Schedule
//=========================================================
Task_t	tlCombatStand1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT_INDEFINITE,		(float)0		},
};

Schedule_t	slCombatStand[] =
{
	{ 
		tlCombatStand1,
		ARRAYSIZE ( tlCombatStand1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_CAN_ATTACK, 
		0,
		"COMBAT_STAND"
	},
};

//=========================================================
// CombatFace Schedule
//=========================================================
Task_t	tlCombatFace1[] =
{
	{ TASK_STOP_MOVING,				0				},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE	},
	{ TASK_FACE_ENEMY,				(float)0		},
};

Schedule_t	slCombatFace[] =
{
	{ 
		tlCombatFace1,
		ARRAYSIZE ( tlCombatFace1 ), 
		bits_COND_CAN_ATTACK			|
		bits_COND_NEW_ENEMY				|
		bits_COND_ENEMY_DEAD,
		0,
		"COMBAT_FACE"
	},
};

//=========================================================
// Standoff schedule. Used in combat when a monster is 
// hiding in cover or the enemy has moved out of sight. 
// Should we look around in this schedule?
//=========================================================
Task_t	tlStandoff[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)2					},
};

Schedule_t slStandoff[] = 
{
	{
		tlStandoff,
		ARRAYSIZE ( tlStandoff ),
		bits_COND_CAN_RANGE_ATTACK1		|
		bits_COND_CAN_RANGE_ATTACK2		|
		bits_COND_ENEMY_DEAD			|
		bits_COND_NEW_ENEMY				|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"STANDOFF"
	}
};

//=========================================================
// Arm weapon (draw gun)
//=========================================================
Task_t	tlArmWeapon[] =
{
	{ TASK_STOP_MOVING,		0				},
	{ TASK_PLAY_SEQUENCE,	(float) ACT_ARM }
};

Schedule_t slArmWeapon[] = 
{
	{
		tlArmWeapon,
		ARRAYSIZE ( tlArmWeapon ),
		0,
		0,
		"ARM_WEAPON"
	}
};

//=========================================================
// reload schedule
//=========================================================
Task_t	tlReload[] =
{
	{ TASK_STOP_MOVING,			0					},
	{ TASK_PLAY_SEQUENCE,		float(ACT_RELOAD)	},
};

Schedule_t slReload[] = 
{
	{
		tlReload,
		ARRAYSIZE ( tlReload ),
		bits_COND_HEAVY_DAMAGE,
		0,
		"RELOAD"
	}
};

//=========================================================
//	Attack Schedules
//=========================================================

// primary range attack
Task_t	tlRangeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slRangeAttack1[] =
{
	{ 
		tlRangeAttack1,
		ARRAYSIZE ( tlRangeAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|	// TODO: may want to selectively disable this per monster
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"RANGE_ATTACK1"
	},
};

// secondary range attack
Task_t	tlRangeAttack2[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_RANGE_ATTACK2,		(float)0		},
};

Schedule_t	slRangeAttack2[] =
{
	{ 
		tlRangeAttack2,
		ARRAYSIZE ( tlRangeAttack2 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|	// TODO: may want to selectively disable this per monster
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"RANGE_ATTACK2"
	},
};

// primary melee attack
Task_t	tlPrimaryMeleeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t	slPrimaryMeleeAttack[] =
{
	{ 
		tlPrimaryMeleeAttack1,
		ARRAYSIZE ( tlPrimaryMeleeAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		|	// TODO: may want to selectively disable this per monster
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED,
		0,
		"MELEE_ATTACK1"
	},
};

// secondary melee attack
Task_t	tlSecondaryMeleeAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_MELEE_ATTACK2,		(float)0		},
};

Schedule_t	slSecondaryMeleeAttack[] =
{
	{ 
		tlSecondaryMeleeAttack1,
		ARRAYSIZE ( tlSecondaryMeleeAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		//bits_COND_LIGHT_DAMAGE		| // TODO: may want to selectively disable this per monster
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED,
		0,
		"MELEE_ATTACK2"
	},
};

// special attack1
Task_t	tlSpecialAttack1[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_SPECIAL_ATTACK1,		(float)0		},
};

Schedule_t	slSpecialAttack1[] =
{
	{ 
		tlSpecialAttack1,
		ARRAYSIZE ( tlSpecialAttack1 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"SPECIAL_ATTACK1"
	},
};

// special attack2
Task_t	tlSpecialAttack2[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_SPECIAL_ATTACK2,		(float)0		},
};

Schedule_t	slSpecialAttack2[] =
{
	{ 
		tlSpecialAttack2,
		ARRAYSIZE ( tlSpecialAttack2 ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"SPECIAL_ATTACK2"
	},
};

// Chase enemy schedule
Task_t tlChaseEnemy1[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CHASE_ENEMY_FAILED	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0		},
	{ TASK_RUN_PATH,			(float)0		},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slChaseEnemy[] =
{
	{ 
		tlChaseEnemy1,
		ARRAYSIZE ( tlChaseEnemy1 ),
		bits_COND_NEW_ENEMY			|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_TASK_FAILED		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"CHASE_ENEMY"
	},
};


// Chase enemy failure schedule
Task_t	tlChaseEnemyFailed[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
//	{ TASK_TURN_LEFT,				(float)179					},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1					},
};

Schedule_t	slChaseEnemyFailed[] =
{
	{ 
		tlChaseEnemyFailed,
		ARRAYSIZE ( tlChaseEnemyFailed ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"CHASE_ENEMY_FAILED"
	},
};


//=========================================================
// small flinch, played when minor damage is taken.
//=========================================================
Task_t tlSmallFlinch[] =
{
	{ TASK_REMEMBER,			(float)bits_MEMORY_FLINCHED },
	{ TASK_STOP_MOVING,			0	},
	{ TASK_SMALL_FLINCH,		0	},
};

Schedule_t slSmallFlinch[] =
{
	{
		tlSmallFlinch,
		ARRAYSIZE ( tlSmallFlinch ),
		0,
		0,
		"SMALL_FLINCH"
	},
};

//=========================================================
// Die!
//=========================================================
Task_t tlDie1[] =
{
	{ TASK_STOP_MOVING,			0				 },
	{ TASK_SOUND_DIE,		(float)0			 },
	{ TASK_DIE,				(float)0			 },
};

Schedule_t slDie[] =
{
	{
		tlDie1,
		ARRAYSIZE( tlDie1 ),
		0,
		0,
		"DIE"
	},
};

//=========================================================
// Victory Dance
//=========================================================
Task_t tlVictoryDance[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_VICTORY_DANCE	},
	{ TASK_WAIT,				(float)0					},
};

Schedule_t slVictoryDance[] =
{
	{
		tlVictoryDance,
		ARRAYSIZE( tlVictoryDance ),
		0,
		0,
		"VICTORY_DANCE"
	},
};

//=========================================================
// BarnacleVictimGrab - barnacle tongue just hit the monster,
// so play a hit animation, then play a cycling pull animation
// as the creature is hoisting the monster.
//=========================================================
Task_t	tlBarnacleVictimGrab[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_PLAY_SEQUENCE,	(float)ACT_BARNACLE_HIT	 },
	{ TASK_SET_ACTIVITY,	(float)ACT_BARNACLE_PULL },
	{ TASK_WAIT_INDEFINITE,	(float)0				 },// just cycle barnacle pull anim while barnacle hoists. 
};

Schedule_t slBarnacleVictimGrab[] =
{
	{
		tlBarnacleVictimGrab,
		ARRAYSIZE ( tlBarnacleVictimGrab ),
		0,
		0,
		"BARNACLE_VICTIM"
	}
};

//=========================================================
// BarnacleVictimChomp - barnacle has pulled the prey to its
// mouth. Victim should play the BARNCLE_CHOMP animation 
// once, then loop the BARNACLE_CHEW animation indefinitely
//=========================================================
Task_t	tlBarnacleVictimChomp[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_PLAY_SEQUENCE,	(float)ACT_BARNACLE_CHOMP },
	{ TASK_SET_ACTIVITY,	(float)ACT_BARNACLE_CHEW  },
	{ TASK_WAIT_INDEFINITE,	(float)0				  },// just cycle barnacle pull anim while barnacle hoists. 
};

Schedule_t slBarnacleVictimChomp[] =
{
	{
		tlBarnacleVictimChomp,
		ARRAYSIZE ( tlBarnacleVictimChomp ),
		0,
		0,
		"BARNACLE_CHOMP"
	}
};


//	Universal Error Schedule
Task_t	tlError[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_WAIT_INDEFINITE,				(float)0 },
};

Schedule_t	slError[] =
{
	{ 
		tlError,
		ARRAYSIZE ( tlError ), 
		0,
		0,
		"ERROR"
	},
};

Task_t tlScriptedWalk[] = 
{
	{ TASK_WALK_TO_TARGET,		(float)TARGET_MOVE_SCRIPTED },
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
	{ TASK_PLANT_ON_SCRIPT,		(float)0		},
	{ TASK_FACE_SCRIPT,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_ENABLE_SCRIPT,		(float)0		},
	{ TASK_WAIT_FOR_SCRIPT,		(float)0		},
	{ TASK_PLAY_SCRIPT,			(float)0		},
};

Schedule_t slWalkToScript[] =
{
	{ 
		tlScriptedWalk,
		ARRAYSIZE ( tlScriptedWalk ),
		SCRIPT_BREAK_CONDITIONS,
		0,
		"WALK_TO_SCRIPT"
	},
};


Task_t tlScriptedRun[] = 
{
	{ TASK_RUN_TO_TARGET,		(float)TARGET_MOVE_SCRIPTED },
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
	{ TASK_PLANT_ON_SCRIPT,		(float)0		},
	{ TASK_FACE_SCRIPT,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_ENABLE_SCRIPT,		(float)0		},
	{ TASK_WAIT_FOR_SCRIPT,		(float)0		},
	{ TASK_PLAY_SCRIPT,			(float)0		},
};

Schedule_t slRunToScript[] =
{
	{ 
		tlScriptedRun,
		ARRAYSIZE ( tlScriptedRun ),
		SCRIPT_BREAK_CONDITIONS,
		0,
		"RUN_TO_SCRIPT"
	},
};

Task_t tlScriptedWait[] = 
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_WAIT_FOR_SCRIPT,		(float)0		},
	{ TASK_PLAY_SCRIPT,			(float)0		},
};

Schedule_t slWaitScript[] =
{
	{ 
		tlScriptedWait,
		ARRAYSIZE ( tlScriptedWait ),
		SCRIPT_BREAK_CONDITIONS,
		0,
		"WAIT_FOR_SCRIPT"
	},
};

Task_t tlScriptedFace[] = 
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_FACE_SCRIPT,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_WAIT_FOR_SCRIPT,		(float)0		},
	{ TASK_PLAY_SCRIPT,			(float)0		},
};

Schedule_t slFaceScript[] =
{
	{ 
		tlScriptedFace,
		ARRAYSIZE ( tlScriptedFace ),
		SCRIPT_BREAK_CONDITIONS,
		0,
		"FACE_SCRIPT"
	},
};

//=========================================================
// Cower - this is what is usually done when attempts
// to escape danger fail.
//=========================================================
Task_t	tlCower[] =
{
	{ TASK_STOP_MOVING,			0					},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_COWER	},
};

Schedule_t	slCower[] =
{
	{
		tlCower,
		ARRAYSIZE ( tlCower ),
		0,
		0,
		"COWER"
	},
};

//=========================================================
// move away from where you're currently standing. 
//=========================================================
Task_t	tlTakeCoverFromOrigin[] =
{
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_ORIGIN,		(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slTakeCoverFromOrigin[] =
{
	{ 
		tlTakeCoverFromOrigin,
		ARRAYSIZE ( tlTakeCoverFromOrigin ), 
		bits_COND_NEW_ENEMY,
		0,
		"TAKE_COVER_FROM_ORIGIN"
	},
};

//=========================================================
// hide from the loudest sound source
//=========================================================
Task_t	tlTakeCoverFromBestSound[] =
{
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slTakeCoverFromBestSound[] =
{
	{ 
		tlTakeCoverFromBestSound,
		ARRAYSIZE ( tlTakeCoverFromBestSound ), 
		bits_COND_NEW_ENEMY,
		0,
		"TAKE_COVER_FROM_BEST_SOUND"
	},
};

//=========================================================
// Take cover from enemy! Tries lateral cover before node 
// cover! 
//=========================================================
Task_t	tlTakeCoverFromEnemy[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_WAIT,					(float)0.2					},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
//	{ TASK_TURN_LEFT,				(float)179					},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1					},
};

Schedule_t	slTakeCoverFromEnemy[] =
{
	{ 
		tlTakeCoverFromEnemy,
		ARRAYSIZE ( tlTakeCoverFromEnemy ), 
		bits_COND_NEW_ENEMY,
		0,
		"TAKE_COVER_FROM_ENEMY"
	},
};


Task_t	tlFollow[] =
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_CANT_FOLLOW },	// If you fail, bail out of follow
	{ TASK_MOVE_TO_TARGET_RANGE,(float)128		},	// Move within 128 of target ent (client)
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_FACE },
};

Schedule_t	slFollow[] =
{
	{
		tlFollow,
		ARRAYSIZE(tlFollow),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FOLLOW"
	},
};


Task_t	tlFaceTarget[] =
{
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_FACE_TARGET,			(float)0		},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_SET_SCHEDULE,		(float)SCHED_TARGET_CHASE },
};

Schedule_t	slFaceTarget[] =
{
	{
		tlFaceTarget,
		ARRAYSIZE(tlFaceTarget),
		bits_COND_CLIENT_PUSH |
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_PROVOKED,
		bits_SOUND_DANGER,
		"FACE_TARGET"
	},
};

Task_t	tlStopFollowing[] =
{
	{ TASK_CANT_FOLLOW,		(float)0 },
};

Schedule_t	slStopFollowing[] =
{
	{
		tlStopFollowing,
		ARRAYSIZE(tlStopFollowing),
		0,
		0,
		"STOP_FOLLOWING"
	},
};

Task_t	tlMoveAway[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_MOVE_AWAY_FAIL },
	{ TASK_STORE_LASTPOSITION,		(float)0		},
	{ TASK_MOVE_AWAY_PATH,			(float)100		},
	{ TASK_WALK_PATH_FOR_UNITS,		(float)100		},
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_FACE_PLAYER,				(float)0.5 },
};

Schedule_t	slMoveAway[] =
{
	{
		tlMoveAway,
		ARRAYSIZE(tlMoveAway),
		0,
		0,
		"MOVE_AWAY"
	},
};


Task_t	tlMoveAwayFail[] =
{
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_FACE_PLAYER,				(float)0.5		},
};

Schedule_t	slMoveAwayFail[] =
{
	{
		tlMoveAwayFail,
		ARRAYSIZE(tlMoveAwayFail),
		0,
		0,
		"MOVE_AWAY_FAIL"
	},
};



Task_t	tlMoveAwayFollow[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_TARGET_FACE },
	{ TASK_STORE_LASTPOSITION,		(float)0		},
	{ TASK_MOVE_AWAY_PATH,			(float)100				},
	{ TASK_WALK_PATH_FOR_UNITS,		(float)100		},
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_SET_SCHEDULE,			(float)SCHED_TARGET_FACE },
};

Schedule_t	slMoveAwayFollow[] =
{
	{
		tlMoveAwayFollow,
		ARRAYSIZE(tlMoveAwayFollow),
		0,
		0,
		"MOVE_AWAY_FOLLOW"
	},
};

Task_t	tlRoam[] =
{
	{ TASK_GET_ROAM_NODE,		(float)0 },
	{ TASK_WALK_PATH,			(float)0 },
	//{ TASK_RUN_PATH,			(float)0 }, // for faster troubleshooting
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0 },
};

Schedule_t	slRoam[] =
{
	{
		tlRoam,
		ARRAYSIZE(tlRoam),
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_FEAR |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_HEAR_SOUND |
		bits_COND_SMELL_FOOD |
		bits_COND_SMELL |
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT |// sound flags
		bits_SOUND_WORLD |
		bits_SOUND_DANGER |

		bits_SOUND_MEAT |// scents
		bits_SOUND_CARCASS |
		bits_SOUND_GARBAGE,
		"ROAM"
	},
};
