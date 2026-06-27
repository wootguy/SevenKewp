/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
#ifndef PLAYER_H
#define PLAYER_H


#include "pm_materials.h"
#include "../dlls/monster/CBaseMonster.h"
#include "usercmd.h"
#include "../dlls/player/player_const.h"
#include "cdll_dll.h"

struct studiohdr_t;

class EXPORT CBasePlayer : public CBaseMonster
{
public:
	
	// Spectator camera
	void	Observer_FindNextPlayer( bool bReverse );
	void	Observer_HandleButtons();
	void	Observer_SetMode( int iMode );
	void	Observer_CheckTarget();
	void	Observer_CheckProperties();
	EHANDLE	m_hObserverTarget;
	EHANDLE m_hActiveCamera; // camera the player is currently viewing
	EHANDLE m_hViewEntity; // entity which the player's view is currently attached to
	float	m_flNextObserverInput;
	int		m_iObserverWeapon;	// weapon of current tracked target
	int		m_iObserverLastMode;// last used observer mode
	bool	m_isObserver;
	float	m_lastObserverSwitch;
	bool m_wantToExitObserver; // set to true if the player should spawn as soon as a spawn point is available
	int		IsObserver() { return m_isObserver; };
	BOOL	IsFirstPerson() { return !m_hViewEntity || m_hViewEntity.GetEdict() == edict(); }
	BOOL	IsBot();
	virtual	BOOL IsNormalMonster(void) { return FALSE; }
	virtual	BOOL IsBarnacleFood(void) { return TRUE; }

	int					random_seed;    // See that is shared between client & server for shared weapons code

	int					m_iPlayerSound;// the index of the sound list slot reserved for this player
	int					m_iTargetVolume;// ideal sound volume. 
	int					m_iWeaponVolume;// how loud the player's weapon is right now.
	int					m_iExtraSoundTypes;// additional classification for this weapon's sound
	int					m_iWeaponFlash;// brightness of the weapon flash
	float				m_flStopExtraSoundTime;
	
	float				m_flFlashLightTime;	// Time until next battery draw/Recharge
	float				m_flFlashLightCarry; // How much time was left before a drain when the flashlight was turned off (prevents infinite flashlight)
	int					m_iFlashBattery;		// Flashlight Battery Draw
	bool				m_flashlightEnabled;
	bool				m_hasFlashlight;
	float				m_lastNightvisionUpdate;
	float				m_lastNightvisionFadeUpdate;
	RGB					m_nightvisionColor;

	int					m_afButtonLast;
	int					m_afButtonPressed;
	int					m_afButtonReleased;
	
	EHANDLE				m_pentSndLast;			// last sound entity to modify player room type
	float				m_flSndRoomtype;		// last roomtype set by sound entity
	float				m_flLastSetRoomtype;		// last roomtype set by sound entity
	float				m_flSndRange;			// dist from player to sound entity
	float				m_DisplacerSndRoomtype;
	Vector				m_DisplacerReturn;

	float				m_flFallVelocity;
	
	int					m_rgItems[MAX_ITEMS];
	int					m_fKnownItem;		// True when a new item needs to be added

	unsigned int		m_afPhysicsFlags;	// physics flags - set when 'normal' physics should be revisited or overriden
	float				m_fNextSuicideTime; // the time after which the player can next use the suicide command


// these are time-sensitive things that we keep track of
	float				m_flTimeStepSound;	// when the last stepping sound was made
	float				m_flTimeWeaponIdle; // when to play another weapon idle animation.
	float				m_flSwimTime;		// how long player has been underwater
	float				m_flDuckTime;		// how long we've been ducking
	float				m_flWallJumpTime;	// how long until next walljump

	float				m_flSuitUpdate;					// when to play next suit update
	int					m_rgSuitPlayList[CSUITPLAYLIST];// next sentencenum to play for suit update
	int					m_iSuitPlayNext;				// next sentence slot for queue storage;
	int					m_rgiSuitNoRepeat[CSUITNOREPEAT];		// suit sentence no repeat list
	float				m_rgflSuitNoRepeatTime[CSUITNOREPEAT];	// how long to wait before allowing repeat
	int					m_lastDamageAmount;		// Last damage taken
	float				m_tbdPrev;				// Time-based damage timer

	float				m_flgeigerRange;		// range to nearest radiation source
	float				m_flgeigerDelay;		// delay per update of range msg to client
	int					m_igeigerRangePrev;
	int					m_iStepLeft;			// alternate left/right foot stepping sound
	char				m_szTextureName[CBTEXTURENAMEMAX];	// current texture name we're standing on
	char				m_chTextureType;		// current texture type

	int					m_idrowndmg;			// track drowning damage taken
	int					m_idrownrestored;		// track drowning damage restored

	int					m_bitsHUDDamage;		// Damage bits for the current fame. These get sent to 
												// the hude via the DAMAGE message
	BOOL				m_fInitHUD;				// True when deferred HUD restart msg needs to be sent
	BOOL				m_fGameHUDInitialized;
	int					m_iTrain;				// Train control position
	BOOL				m_fWeapon;				// Set this to FALSE to force a reset of the current weapon HUD info

	EHANDLE				m_pTank;				// the tank which the player is currently controlling,  NULL if no tank
	EHANDLE				m_pPushable;			// the pushable which the player is currently lifting,  NULL if no tank
	float				m_fDeadTime;			// the time at which the player died  (used in PlayerDeathThink())

	BOOL			m_fNoPlayerSound;	// a debugging feature. Player makes no sound if this is true. 
	BOOL			m_fLongJump; // does this player have the longjump module?

	float       m_tSneaking;
	int			m_iUpdateTime;		// stores the number of frame ticks before sending HUD update messages
	int			m_iClientHealth;	// the health currently known by the client.  If this changes, send a new
	int			m_iClientBattery;	// the Battery currently known by the client.  If this changes, send a new
	int			m_iHideHUD;		// the players hud weapon info is to be hidden
	bool		m_fakeSuit; // tell client they have a suit, even if they don't, so they can switch weapons
	int			m_iClientHideHUD;
	int			m_iFOV;			// field of view
	int			m_iClientFOV;	// client's known FOV
	// usable player items 
#ifdef CLIENT_DLL
	CBasePlayerItem* m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem* m_pActiveItem;
	CBasePlayerItem* m_pClientActiveItem;  // client version of the active item
	CBasePlayerItem* m_pLastItem;
#else
	EHANDLE m_rgpPlayerItems[MAX_ITEM_TYPES];
	EHANDLE m_pActiveItem;
	EHANDLE m_pClientActiveItem;  // client version of the active item
	EHANDLE m_pLastItem;
#endif
	// shared ammo slots
	int	m_rgAmmo[MAX_AMMO_SLOTS];
	int	m_rgAmmoLast[MAX_AMMO_SLOTS];

	Vector				m_vecAutoAim;
	BOOL				m_fOnTarget;
	int					m_iDeaths;
	float				m_flRespawnTimer; // used in PlayerDeathThink() to make sure players can always respawn;

	int m_lastx, m_lasty;  // These are the previous update's crosshair angles, DON"T SAVE/RESTORE

	int m_nCustomSprayFrames;// Custom clan logo frames for this player
	float	m_flNextDecalTime;// next time this player can spray a decal
	float	m_lastDropTime; // last time player dropped a weapon
	float	m_nextItemPickups[MAX_WEAPONS]; // next time player can pick up a weapon

	char m_szTeamName[TEAM_NAME_LENGTH];

	unsigned char* m_lastPvs; // only valid during AddToFullPack
	unsigned char* m_lastPas; // only valid during AddToFullPack

	bool m_headshot; // last TakeDamage was a headshot
	Vector m_headshotDir; // direction of headshot
	Vector m_weaponRecoil; // additional punch angle from weapon events

	float m_lastInteractMessage; // last time an interaction message was sent to this player
	bool m_droppedDeathWeapons;

	float m_scoreMultiplier;
	
	// for calculating idle time
	int m_lastUserButtonState;
	float m_lastUserInput;
	int m_lastScoreStatus; // last status sent to player scoreboards

	float m_useKeyTime; // when the use key was last pressed. 0 if not currently pressed
	float m_nextAntiBlock; // next time player is allowed to swap
	bool m_useExpired; // use was held for too long. Player must release and press the key again to use
	bool m_usingMomentary; // player activated a momentary button any time during this key press

	int m_lastPacketEnts; // number of packet entities sent in the previous frame
	float m_newPacketEnts; // number of newly visible packet entities sent recently. Decremented over time.
	float m_lastDeltaUpdate; // last time entity deltas were sent

	string_t m_playerModelName;
	studiohdr_t* m_playerModel; // raw player model data (NULL if not installed on the server)
	int m_playerModelSize; // file size in bytes
	PLAYER_MODEL_ANIM_SET m_playerModelAnimSet;

	bool m_isBarnacleFood; // player is being eaten after being pulled up to the barnacle

	uint32_t m_debugFlags; // misc flags for developers and mappers
	float m_lastNodeUpdate;

	// currently held weapons
	uint64_t m_weaponBits;
	uint64_t m_lastWeaponBits;

	// updates for frequently updated details like HP
	float m_lastTagUpdate;
	uint16_t m_lastTagHp;
	uint16_t m_lastTagMaxHp;
	uint16_t m_lastTagArmor;
	uint8_t m_lastTagObserver;

	usercmd_t m_lastCmd;
	uint32_t m_cmdTime; // time accumulated from user commands, syncronized with the client

	bool m_godmode; // godmode cheat enabled
	bool m_noclip; // noclip cheat enabled
	bool m_notarget; // no-target cheat enabled
	bool m_instakill; // instakill cheat enabled

	string_t m_playerModelOverride; // player model forced by the map

	virtual void Spawn( void );

//	virtual void Think( void );
	virtual void Jump( void ) STUB_VOID;
	virtual void Duck( void ) STUB_VOID;
	virtual void PreThink( void ) STUB_VOID;
	virtual void PostThink( void ) STUB_VOID;
	virtual Vector GetGunPosition( void );
	virtual int TakeHealth( float flHealth, int bitsDamageType, float healthcap=0) STUB_INT;
	virtual void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType) STUB_VOID;
	virtual int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) STUB_INT;
	virtual void	Killed( entvars_t *pevAttacker, int iGib );
	virtual Vector BodyTarget(const Vector& posSrc) STUB_VEC;
	virtual void StartSneaking(void) STUB_VOID;
	virtual void StopSneaking(void) STUB_VOID;
	virtual BOOL IsSneaking(void) STUB_INT;
	virtual BOOL ShouldFadeOnDeath( void ) { return FALSE; }
	virtual	BOOL IsPlayer( void ) { return TRUE; }			// Spectators should return FALSE for this, they aren't "players" as far as game logic is concerned
	virtual CBasePlayer* MyPlayerPointer(void) { return this; };

	virtual BOOL IsNetClient( void ) { return TRUE; }		// Bots should return FALSE for this, they can't receive NET messages
															// Spectators should return TRUE for this
	virtual const char *TeamID( void ) STUB_INT;
	virtual const char* DisplayName() { return STRING(pev->netname); }
	virtual int		Save( CSave &save ) STUB_INT;
	virtual int		Restore( CRestore &restore ) STUB_INT;
	void RenewItems(void);
	void PackDeadPlayerItems( void );
	void HideAllItems(bool hideSuit);
	
	// if removeItemsOnly=true, then remove items server-side, but don't update the client hud
	// (fixes race condition during player spawn)
	void RemoveAllItems( BOOL removeSuit, BOOL removeItemsOnly=false );
	BOOL SwitchWeapon( CBasePlayerItem *pWeapon ) STUB_INT;

	// JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	virtual void UpdateClientData( void ) STUB_VOID;

	void SendWeaponList();

	void ReloadHUD();

	// if fast, then only send essential user info because it will be reset shortly
	void Rename(const char* newName, bool fast, edict_t* dst = NULL);

	// replaces player model in user info
	char* ReplaceModelUserInfo(char* userinfo, const char* newModel);

	void SetMapPlayerModel(const char* newModel);

	void SetPrefsFromUserinfo(char* infobuffer);
	
	static	TYPEDESCRIPTION m_playerSaveData[];

	// Player is moved across the transition by other means
	virtual int		ObjectCaps( void ) { return CBaseMonster :: ObjectCaps() & ~(FCAP_IMPULSE_USE | FCAP_ACROSS_TRANSITION); }
	virtual void	Precache( void ) STUB_VOID;
	BOOL			IsOnLadder( void );
	BOOL			FlashlightIsOn( void );
	void			FlashlightTurnOn( void );
	void			FlashlightTurnOff( void );
	
	void UpdatePlayerSound ( void );
	void DeathSound ( void ) STUB_VOID;

	int Classify ( void ) STUB_INT;
	void SetAnimation( PLAYER_ANIM playerAnim, float duration=0 ) STUB_VOID;
	void SetWeaponAnimType( const char *szExtention );
	char m_szAnimExtention[32];
	char m_szAnimAction[32]; // "aim" or "hold"

	// custom player functions
	virtual void ImpulseCommands( void ) STUB_VOID;
	void CheatImpulseCommands( int iImpulse );

	void StartDeathCam( void );
	void StartObserver( Vector vecPosition, Vector vecViewAngle );
	void LeaveObserver(bool respawn=true);

	void AddPoints( int score, BOOL bAllowNegativeScore ) STUB_VOID;
	void AddPointsToTeam( int score, BOOL bAllowNegativeScore ) STUB_VOID;
	BOOL AddPlayerItem( CBasePlayerItem *pItem ) STUB_INT;
	BOOL RemovePlayerItem( CBasePlayerItem *pItem ) STUB_INT;
	void DropPlayerItem ( const char *pszItemName );
	void DropAmmo(bool secondary);
	BOOL HasPlayerItem( CBasePlayerItem *pCheckItem );
	CBasePlayerItem* GetNamedPlayerItem(const char* pszItemName);
	CBasePlayerItem* GetPlayerItemById(int id);
	BOOL HasNamedPlayerItem( const char *pszItemName );
	BOOL HasWeapons( void );// do I have ANY weapons?
	void SelectPrevItem( int iItem );
	void SelectNextItem( int iItem );
	void SelectLastItem(void);
	void SelectItem(const char *pstr);
	void ItemPreFrame( void );
	void ItemPostFrame( void );
	void GiveNamedItem( const char *szName );
	void EnableControl(BOOL fControl);
	void DisableWeapons(bool disable);

	int  GiveAmmo( int iAmount, const char *szName ) STUB_INT;
	void SendAmmoUpdate(void);

	void WaterMove( void );
	void PlayerDeathThink( void );
	void PlayerUse( void );

	void CheckSuitUpdate();
	void SetSuitUpdate(const char *name, int fgroup, int iNoRepeat) STUB_VOID;
	void UpdateGeigerCounter( void );
	void CheckTimeBasedDamage( void );

	BOOL BarnacleVictimCaught( void ) STUB_INT;
	void BarnacleVictimBitten ( entvars_t *pevBarnacle ) STUB_VOID;
	void BarnacleVictimReleased ( void ) STUB_VOID;
	static int GetAmmoIndex(const char *psz);
	int AmmoInventory( int iAmmoIndex );
	int Illumination( void ) STUB_INT;

	void ResetAutoaim( void );
	Vector GetAutoaimVector( float flDelta  );
	Vector AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  );
	virtual Vector GetLookDirection() STUB_VEC;

	void ForceClientDllUpdate( void );  // Forces all client .dll specific data to be resent to client.

	void DeathMessage( entvars_t *pevKiller );

	void SetCustomDecalFrames( int nFrames );
	int GetCustomDecalFrames( void );

	void CleanupWeaponboxes(void);

	void TabulateWeapons( void ) STUB_VOID; // initializes weapons hud for client
	void TabulateAmmo( void ) STUB_VOID;
	int rgAmmo(int ammoIdx);
	void rgAmmo(int ammoIdx, int newCount);

	// stop control over tanks and pushables
	void ReleaseControlledObjects();

	float m_flStartCharge;
	float m_flAmmoStartCharge;
	float m_flPlayAftershock;
	float m_flNextAmmoBurn;// while charging, when to absorb another unit of player's ammo?
	
	//Player ID
	void InitStatusBar( void );
	void UpdateStatusBar( void );
	int m_izSBarState[ SBAR_END ];
	float m_flNextSBarUpdateTime;
	float m_flStatusBarDisappearDelay;
	char m_SbarString0[ SBAR_STRING_SIZE ];
	char m_SbarString1[ SBAR_STRING_SIZE ];
	int tempNameActive; // +1 for each status bar update while the player's name/team is currently swapped for status bar coloring
	char m_tempName[SBAR_STRING_SIZE];
	int m_tempTeam;

	float m_flNextChatTime;
	
	int m_iAutoWepSwitch;
	int m_iWantClassSelection;	// set whenever userinfo changes
	int m_iClassSelection;		// synced to desired class on spawn
	bool m_allowMapPlayerModels; // true if player is ok with maps changing their player model
	int m_nametags;

	float m_lastScoreUpdate;
	float m_lastTimeLeftUpdate;
	char m_lastNextMap[MAX_MAP_NAME];
	int m_lastScore;
	void UpdateScore();

	void UpdateTag(CBasePlayer* dst=NULL);

	int16_t m_lastTagPos[32][3];
	float m_lastTagPosUpdate;
	void UpdateTagPos();

	uint16_t GetScoreboardStatus();
	void UpdateTeamInfo(int color=-1, int msg_mode=MSG_ALL, edict_t* dst=NULL);

	float m_lastSpawnMessage;
	bool m_deathMessageSent;
	bool m_allowFriendlyFire; // true if this player allows incoming friendly fire

	float m_extraRespawnDelay; // set to non-zero to increase respawn delay (sums with map default)
	float m_tempRespawnDelay; // set to non-zero to increase respawn delay (resets on spawn)
	bool m_reviveAttempted; // someone attempted to revive this player while dead

	float m_initSoundTime;
	
	HL_CLIENT_ENGINE_VERSION m_clientEngineVersion; // which game engine is the is this player using?
	HL_CLIENT_MOD_VERSION m_clientModVersion; // which mod is this player using?
	HL_CLIENT_RENDERER m_clientRenderer;
	HL_CLIENT_SYSTEM m_clientSystem;
	string_t m_clientModVersionString; // version string for the client mod
	int m_sevenkewpVersion; // version number for this mod's client
	bool m_sentClientWarning; // has this client been warned about their client incompatability?
	bool m_sentSevenKewpNotice; // has this client been told about the sevenkewp client?
	bool m_clientCheckFinished;

	string_t m_queryResults[6]; // one for each request in QueryClientType

	int GetNameColor();

	const char* GetTeamName();

	// checks client cvars to determine which engine and mod is being used. Called when the player first enters the server.
	void QueryClientType();

	// we know which client the player is using now (probably)
	void QueryClientTypeFinished();

	void HandleClientCvarResponse(int requestID, const char* pszCvarName, const char* pszValue);

	client_info_t GetClientInfo();

	void SendLegacyClientWarning();

	void SendSevenKewpClientNotice(const char* weaponName);

	const char* GetClientVersionString();

	bool IsSevenKewpClient() STUB_INT;

	bool UseSevenKewpGuns() STUB_INT; // true if the player wants sevenkewp guns and is using the client

	// gets legacy steam id (e.g. STEAM_0:0:12345679)
	const char* GetSteamID();

	// gets steam community id (e.g. 123456789901234523)
	uint64_t GetSteamID64();

	// get ID assigned by the server (starts at 1, and every new player increments this by 1)
	int GetUserID();

	// show a message to help the understand what is happening in the game
	// e.g. Why they can't pick up an item or have a monster follow them
	void ShowInteractMessage(const char* msg);

	// if death=true, only drop items that are not marked to keep on death
	// if respawn=true, only drop items that are not marked to keep on respawn
	// if forceDrop=true, force drop all items ignoring any "can't drop" rules
	// returns false if not all inventory items were dropped due to restrictions
	bool DropAllInventoryItems(bool deathDrop = false, bool respawnDrop = false, bool forceDrop = false);

	virtual void Revive() STUB_VOID;

	float GetDamage(float defaultDamage) STUB_INT;

	// accounts for active cameras and view offset
	Vector GetViewPosition();

	// for scoring
	void PenalizeDeath();

	// -1 deaths = use current death counter
	float GetScoreMultiplier(int deaths = -1);

	// how long has it been since the player last pressed any buttons or typed in chat
	float GetIdleTime();
	
	// how long the use key has been held down for
	float GetUseTime();

	// get look direction snapped to the closest X, Y, or Z axis
	Vector GetSnappedLookDir();

	int GetTraceHull();

	CBaseEntity* AntiBlockTrace();

	// 0 = no target, 1 = swapped, 2 = swap failed with error message
	int TryAntiBlock();

	// save score to global state
	void SaveScore();

	// load score from global state, or initialize to 0
	void LoadScore();

	// save inventory to global state
	void SaveInventory();

	// equip inventory from global state
	// returns false if no inventory is saved for this player
	bool LoadInventory();

	// tell the client which weapon belongs in a slot which multiple weapons can fill
	void ResolveWeaponSlotConflict(int wepId);

	// WeaponList msgs reset clip counts to 0. Those messages are used to resolve conflicts
	// in slots that multiple weapons share. Call this to fix the clip counts and unselectable
	// weapons in the HUD for the client.
	// thisIdOnly = only fix clip count for the weapons that share this ID's slot
	void FixSharedWeaponSlotClipCount(int thisIdOnly=-1);

	// if a weapon slot can be filled by multiple weapons, this returns the weapon ID
	// that is currently held in that slot. If no weapon is filling the slot
	// that queryWepId fills, then -1 is returned.
	// queryWepId can be any weapon that fills the slot in question
	int GetCurrentIdForConflictedSlot(int queryWepId);

	const char* GetDeathNoticeWeapon() STUB_INT;

	void NightvisionUpdate();

	virtual void SetClassification(int newClass) override { CBaseEntity::SetClassification(newClass); }
	
	void ResetSequenceInfo() override STUB_VOID;

	// syncs upper and lower body animations for a custom model
	// gaitSpeed = 2D movement speed
	// defaultSyncMultiplier = multiplier that syncs the upper/lower body on player.mdl
	void SyncGaitAnimations(int animDesired, float gaitSpeed, float defaultSyncMultiplier);

	// loads player model data and triggers plugin hooks. Conditionally broadcasts info to other players.
	void PlayerModelChanged(const char* newModel, bool broadcast=true);

	// send current userinfo to all players (name, model, etc.)
	void BroadcastUserInfo();

	void DebugThink();

	void SetThirdPersonWeaponAnim(int sequence, float fps=1.0f) STUB_VOID;

	// 0 = default velocity (800)
	// -1 = jumping disabled
	// 1+ = custom velocity
	void SetJumpPower(int power) STUB_VOID;

	void SyncWeaponBits();

	bool HasSuit();

	// make splashes from player actions (call MakeVectors before this)
	void WaterSplashTrace(Vector vecSrc, float dist, int hull, float scale);

	void ApplyEffects() STUB_VOID;

	// for sven-style monster info
	//void UpdateMonsterInfo();
	//float m_lastMonsterInfoMsg;
	//float m_lastMonsterInfoTrace;
	//char m_lastMonsterInfoText[128];
};

#define AUTOAIM_2DEGREES  0.0348994967025
#define AUTOAIM_5DEGREES  0.08715574274766
#define AUTOAIM_8DEGREES  0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669


EXPORT extern int	gmsgHudText;
EXPORT extern BOOL gInitHUD;

#endif // PLAYER_H
