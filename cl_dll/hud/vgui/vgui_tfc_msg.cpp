#include "hud.h"
#include "parsemsg.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ScorePanel.h"
#include "shared_util.h"
#include "cl_util.h"

int TeamFortressViewport::MsgFunc_ValClass(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	for (int i = 0; i < 5; i++)
		m_iValidClasses[i] = READ_SHORT();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_TeamNames(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iNumberOfTeams = READ_BYTE();

	for (int i = 0; i < m_iNumberOfTeams; i++)
	{
		int teamNum = i + 1;

		gHUD.m_TextMessage.LocaliseTextString(READ_STRING(), m_sTeamNames[teamNum], MAX_TEAMNAME_SIZE);

		// Set the team name buttons
		if (m_pTeamButtons[i])
			m_pTeamButtons[i]->setText(m_sTeamNames[teamNum]);

		// range check this value...m_pDisguiseButtons[5];
		if (teamNum < 5)
		{
			// Set the disguise buttons
			if (m_pDisguiseButtons[teamNum])
				m_pDisguiseButtons[teamNum]->setText(m_sTeamNames[teamNum]);
		}
	}

	// Update the Team Menu
	if (m_pTeamMenu)
		m_pTeamMenu->Update();

	return 1;
}

int TeamFortressViewport::MsgFunc_Feign(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iIsFeigning = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_Detpack(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iIsSettingDetpack = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_VGUIMenu(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iMenu = READ_BYTE();

	// Map briefing includes the name of the map (because it's sent down before the client knows what map it is)
	if (iMenu == MENU_MAPBRIEFING)
	{
		strncpy(m_sMapName, READ_STRING(), sizeof(m_sMapName));
		m_sMapName[sizeof(m_sMapName) - 1] = '\0';
	}

	// Bring up the menu6
	ShowVGUIMenu(iMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_MOTD(const char* pszName, int iSize, void* pbuf)
{
	if (m_iGotAllMOTD)
		m_szMOTD[0] = 0;

	BEGIN_READ(pbuf, iSize);

	m_iGotAllMOTD = READ_BYTE();

	int roomInArray = sizeof(m_szMOTD) - strlen(m_szMOTD) - 1;

	strncat(m_szMOTD, READ_STRING(), roomInArray >= 0 ? roomInArray : 0);
	m_szMOTD[sizeof(m_szMOTD) - 1] = '\0';

	// don't show MOTD for HLTV spectators
	if (m_iGotAllMOTD && !gEngfuncs.IsSpectateOnly())
	{
		ShowVGUIMenu(MENU_INTRO);
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_BuildSt(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iBuildState = READ_SHORT();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	return 1;
}

int TeamFortressViewport::MsgFunc_RandomPC(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iRandomPC = READ_BYTE();

	return 1;
}

int TeamFortressViewport::MsgFunc_ServerName(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	strncpy(m_szServerName, READ_STRING(), sizeof(m_szServerName));
	m_szServerName[sizeof(m_szServerName) - 1] = 0;

	return 1;
}

int TeamFortressViewport::MsgFunc_NextMap(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	strcpy_safe(m_szNextMap, READ_STRING(), sizeof(m_szNextMap));
	return 1;
}

int TeamFortressViewport::MsgFunc_TimeLeft(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	m_timeLeft = READ_LONG();
	return 1;
}

int TeamFortressViewport::MsgFunc_ScoreInfo(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	short cl = READ_BYTE();
	short frags = READ_SHORT();
	short deaths = READ_SHORT();
	short playerclass = READ_SHORT();
	short teamnumber = READ_SHORT();

	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		g_PlayerExtraInfo[cl].frags = frags;
		g_PlayerExtraInfo[cl].deaths = deaths;
		g_PlayerExtraInfo[cl].playerclass = playerclass;
		g_PlayerExtraInfo[cl].teamnumber = teamnumber;

		//Dont go bellow 0!
		if (g_PlayerExtraInfo[cl].teamnumber < 0)
			g_PlayerExtraInfo[cl].teamnumber = 0;

		UpdateOnPlayerInfo();
	}

	return 1;
}
// Message handler for TeamScore message
// accepts three values:
//		string: team name
//		short: teams kills
//		short: teams deaths 
// if this message is never received, then scores will simply be the combined totals of the players.
int TeamFortressViewport::MsgFunc_TeamScore(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	char* TeamName = READ_STRING();

	// find the team matching the name
	int i;
	for (i = 1; i <= m_pScoreBoard->m_iNumTeams; i++)
	{
		if (!stricmp(TeamName, g_TeamInfo[i].name))
			break;
	}

	if (i > m_pScoreBoard->m_iNumTeams)
		return 1;

	// use this new score data instead of combined player scoresw
	g_TeamInfo[i].scores_overriden = TRUE;
	g_TeamInfo[i].frags = READ_SHORT();
	g_TeamInfo[i].deaths = READ_SHORT();

	return 1;
}

// Message handler for TeamInfo message
// accepts two values:
//		byte: client number
//		string: client team name
int TeamFortressViewport::MsgFunc_TeamInfo(const char* pszName, int iSize, void* pbuf)
{
	if (!m_pScoreBoard)
		return 1;

	BEGIN_READ(pbuf, iSize);
	short cl = READ_BYTE();

	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		// set the players team
		strncpy(g_PlayerExtraInfo[cl].teamname, READ_STRING(), MAX_TEAM_NAME);
	}

	// rebuild the list of teams
	m_pScoreBoard->RebuildTeams();

	return 1;
}

int TeamFortressViewport::MsgFunc_Spectator(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	short cl = READ_BYTE();
	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		g_IsSpectator[cl] = READ_BYTE();
	}

	return 1;
}

int TeamFortressViewport::MsgFunc_AllowSpec(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	m_iAllowSpectators = READ_BYTE();

	// Force the menu to update
	UpdateCommandMenu(m_StandardMenu);

	// If the team menu is up, update it too
	if (m_pTeamMenu)
		m_pTeamMenu->Update();

	return 1;
}

#if defined( _TFC )
const Vector& GetTeamColor(int team_no);
extern globalvars_t* gpGlobals;
#endif 

// used to reset the player's screen immediately
int TeamFortressViewport::MsgFunc_ResetFade(const char* pszName, int iSize, void* pbuf)
{
#if defined( _TFC )
	if (!gpGlobals)
		return 0;

	screenfade_t sf;
	gEngfuncs.pfnGetScreenFade(&sf);

	sf.fader = 0;
	sf.fadeg = 0;
	sf.fadeb = 0;
	sf.fadealpha = 0;

	sf.fadeEnd = 0.1;
	sf.fadeReset = 0.0;
	sf.fadeSpeed = 0.0;

	sf.fadeFlags = FFADE_IN;

	sf.fadeReset += gpGlobals->time;
	sf.fadeEnd += sf.fadeReset;

	gEngfuncs.pfnSetScreenFade(&sf);
#endif

	return 1;
}

// used to fade a player's screen out/in when they're spectating someone who is teleported
int TeamFortressViewport::MsgFunc_SpecFade(const char* pszName, int iSize, void* pbuf)
{
#if defined( _TFC )
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();

	// we're in first-person spectator mode (...not first-person in the PIP)
	if (g_iUser1 == OBS_IN_EYE)
	{
		// this is the person we're watching
		if (g_iUser2 == iIndex)
		{
			int iFade = READ_BYTE();
			int iTeam = READ_BYTE();
			float flTime = ((float)READ_SHORT() / 100.0);
			int iAlpha = READ_BYTE();

			Vector team = GetTeamColor(iTeam);

			screenfade_t sf;
			gEngfuncs.pfnGetScreenFade(&sf);

			sf.fader = team[0];
			sf.fadeg = team[1];
			sf.fadeb = team[2];
			sf.fadealpha = iAlpha;

			sf.fadeEnd = flTime;
			sf.fadeReset = 0.0;
			sf.fadeSpeed = 0.0;

			if (iFade == BUILD_TELEPORTER_FADE_OUT)
			{
				sf.fadeFlags = FFADE_OUT;
				sf.fadeReset = flTime;

				if (sf.fadeEnd)
					sf.fadeSpeed = -(float)sf.fadealpha / sf.fadeEnd;

				sf.fadeTotalEnd = sf.fadeEnd += gpGlobals->time;
				sf.fadeReset += sf.fadeEnd;
			}
			else
			{
				sf.fadeFlags = FFADE_IN;

				if (sf.fadeEnd)
					sf.fadeSpeed = (float)sf.fadealpha / sf.fadeEnd;

				sf.fadeReset += gpGlobals->time;
				sf.fadeEnd += sf.fadeReset;
			}

			gEngfuncs.pfnSetScreenFade(&sf);
		}
	}
#endif

	return 1;
}

#define CUSTOM_MESSAGES() \
    X(ValClass)		\
    X(TeamNames)	\
    X(Feign)		\
    X(Detpack)		\
    X(MOTD)			\
    X(BuildSt)		\
    X(RandomPC)		\
    X(ServerName)	\
    X(NextMap)		\
    X(TimeLeft)		\
    X(ScoreInfo)	\
    X(TeamScore)	\
    X(TeamInfo)		\
    X(Spectator)	\
    X(AllowSpec)	\
    X(SpecFade)		\
    X(ResetFade)	\
    X(VGUIMenu)		\

#define X(x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
	{ return gViewPort->MsgFunc_##x(pszName, iSize, pbuf ); }
CUSTOM_MESSAGES()
#undef X

void HookTfcViewportMessages() {
#define X HOOK_MESSAGE
	CUSTOM_MESSAGES()
#undef X
}