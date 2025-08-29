#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h> // _alloca

#include "event_api.h"
#include "pm_defs.h"
#include "pm_shared.h"

extern Vector v_origin;
extern Vector v_angles;
extern vec3_t cam_ofs;
extern int cam_thirdperson;

extern hud_player_info_t	 g_PlayerInfoList[MAX_PLAYERS + 1];	   // player info from the engine
extern extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS + 1];

float* GetClientColor(int clientIndex);


int CHudNametags::Init(void)
{
	gHUD.AddHudElem(this);

	m_HUD_nametags = gEngfuncs.pfnRegisterVariable("cl_nametags", "1", FCVAR_ARCHIVE | FCVAR_USERINFO);
	m_HUD_nametag_hp = gEngfuncs.pfnRegisterVariable("cl_nametags_hp", "1", FCVAR_ARCHIVE);

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE; // is always drawn during an intermission

	return 1;
}

Vector WorldToScreen(const Vector& P, const Vector& CamPos, const Vector& anglesDeg, float fovXDeg) {
    Vector forward, right, up;
    AngleVectors(anglesDeg, forward, right, up);

    Vector rel = P - CamPos;

    float x = DotProduct(rel, right);
    float y = DotProduct(rel, up);
    float z = DotProduct(rel, forward);

    int screenW = ScreenWidth;
    int screenH = ScreenHeight;
    float aspect = (float)screenW / (float)screenH;

    // convert to actual horizontal FOV for this aspect
    float baseAspect = 4.0f / 3.0f;
    float fovXRad43 = fovXDeg * (M_PI / 180.0f);
    float fovXRad = 2.0f * atan(tan(fovXRad43 * 0.5f) * (aspect / baseAspect));
    float fovYRad = 2.0f * atan(tan(fovXRad * 0.5f) / aspect);
    float f = 1.0f / tan(fovYRad * 0.5f);

    float ndcX = (x * f / aspect) / z;
    float ndcY = (y * f) / z;

    float screenX = (ndcX + 1.0f) * 0.5f * screenW;
    float screenY = (1.0f - ndcY) * 0.5f * screenH;

    return { screenX, screenY, z };
}

int CHudNametags::Draw(float flTime)
{
	if (m_HUD_nametags->value <= 0)
		return 0;
    
    if (!gHUD.IsSevenKewpServer())
        return 0; // no cheating in hldm

	cl_entity_t* localPlayer = GetLocalPlayer();

    Vector angles = v_angles;
    
    if (cam_thirdperson) {
        angles.x = cam_ofs.x;
        angles.y = cam_ofs.y;
    }

    float fov = gHUD.m_iFOV;

    static Vector targetOri[32];
    static Vector lastOri[32];
    static float lastFrame;
    static float lastLerp;
    const float lerpTime = 0.1f;

    float now = gEngfuncs.GetClientTime();

    float frameTime = clamp(now - lastFrame, 0, 0.5f);
    lastFrame = now;

    bool xray = m_HUD_nametags->value >= 2;

    int screenW = ScreenWidth;
    int screenH = ScreenHeight;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		cl_entity_s* pClient = gEngfuncs.GetEntityByIndex(i + 1);
        extra_player_info_t& info = g_PlayerExtraInfo[i + 1];

        if (!pClient || !g_PlayerInfoList[i + 1].name)
            continue;

        bool clientVisible = pClient->curstate.messagenum >= localPlayer->curstate.messagenum;

		// Don't show an icon if the player is not in our PVS.
		if (!clientVisible && !xray)
			continue;

		// Don't show an icon for spectators
		if (info.specMode && info.specMode != OBS_ROAMING)
			continue;

		// Don't show an icon for the local player unless we're in thirdperson mode.
		if (pClient == localPlayer && !cam_thirdperson)
			continue;
		
        // lerp between inaccurate positions for smooth movement
        float lerpt = (now - lastLerp) / lerpTime;
        Vector lerpori = lastOri[i] + (targetOri[i] - lastOri[i]) * lerpt;

        Vector ori = clientVisible ? pClient->origin : lerpori;
		Vector tagOri = ori + Vector(0, 0, 40);

        bool canSeePlayer = false;

        if (clientVisible) {
            pmtrace_t tr;
            //gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
            //gEngfuncs.pEventAPI->EV_PushPMStates();
            gEngfuncs.pEventAPI->EV_SetTraceHull(2);

            Vector testOris[3] = {
                tagOri,
                ori,
                ori + Vector(0, 0, -32)
            };

            for (int i = 0; i < 3; i++) {
                gEngfuncs.pEventAPI->EV_PlayerTrace(v_origin, testOris[i], PM_STUDIO_IGNORE | PM_GLASS_IGNORE, -1, &tr);
                if (tr.fraction >= 1.0f) {
                    canSeePlayer = true;
                    break;
                }
            }

            if (m_HUD_nametags->value < 2 && !canSeePlayer) {
                continue;
            }
        }

        Vector screenOri = WorldToScreen(tagOri, v_origin, angles, fov);

        //int m_iBeam = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/smoke.spr");
        //gEngfuncs.pEfxAPI->R_BeamPoints(headOri, v_origin, m_iBeam, 1, 1, 0, 1, 0, 0, 0, 0, 1, 1);

        const char* name = g_PlayerInfoList[i + 1].name;
        int nameWidth, nameHeight;
        GetConsoleStringSize(name, &nameWidth, &nameHeight);
        
        const char* hpStr = "";
        int hp = info.health;
        int hpWidth = 0;
        int hpHeight = 0;
        
        if (info.specMode || m_HUD_nametag_hp->value == 0) {
            hpStr = "";
            hpWidth = 0;
        }
        else {
            hpStr = hp ? UTIL_VarArgs(" %d%%", (int)hp) : " DEAD";
            GetConsoleStringSize(hpStr, &hpWidth, &hpHeight);
        }

        int tagWidth = nameWidth + hpWidth;
        int tagHeight = nameHeight * 2;

        int x = screenOri.x - tagWidth * 0.5f;
        int y = screenOri.y - nameHeight;
        
        if (x > screenW || y > screenH || x + tagWidth < 0 || y + tagHeight < 0 || screenOri.z < 0.01f) {
            continue;
        }
        /*
        if (m_HUD_nametags->value < 3) {
            if (x > screenW || y > screenH || x + tagWidth < 0 || y + tagHeight < 0 || screenOri.z < 0.01f) {
                continue;
            }
        }
        else {
            if (x < 0)
                x = 0;
            else if (x + tagWidth > screenW)
                x = screenW - tagWidth;

            if (y + tagHeight > screenH)
                y = screenH - tagHeight;
            else if (y < 0)
                y = 0;
        }
        */

        float* color = GetClientColor(i+1);
        RGB nameColor(color[0]*255, color[1]*255, color[2]*255);

        DrawConsoleString(x, y, name, nameColor);

        if (hpWidth) {
            RGB hpColor(255, 255, 255);
            if (hp <= 25)       hpColor = RGB(255, 0, 0);
            else if (hp <= 50)  hpColor = RGB(255, 128, 0);
            else if (hp <= 75)  hpColor = RGB(255, 255, 0);

            DrawConsoleString(x + nameWidth, y, hpStr, hpColor);
        }

        // show distance to invisible players
        if (!canSeePlayer) {
            int meters = V_max(0, (v_origin - tagOri).Length() / 33);
            const char* dist = UTIL_VarArgs("(%dm)", meters);
            int distWidth;
            GetConsoleStringSize(dist, &distWidth, &nameHeight);

            RGB distColor(160, 160, 160);
            x = screenOri.x - distWidth * 0.5f;
            DrawConsoleString(x, y + nameHeight, dist, distColor);
        }
	}

    // update lerp to/from positions
    if (now - lastLerp >= lerpTime || lastLerp > now) {
        lastLerp = now;

        memcpy(lastOri, targetOri, sizeof(lastOri));

        for (int i = 0; i < MAX_CLIENTS; i++) {
            extra_player_info_t& info = g_PlayerExtraInfo[i + 1];
            targetOri[i] = Vector(info.x, info.y, info.z);
        } 
    }
	
	return 1;
}
