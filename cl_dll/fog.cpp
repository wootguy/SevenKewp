#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include "engine_pv.h"
#include "GL/gl.h"
#include "triangleapi.h"

DECLARE_MESSAGE(m_Fog, Fog);

int CFog::Init(void)
{
	HOOK_MESSAGE(Fog);

	return 1;
}

int CFog::VidInit(void) {
	enabled = false;

	return 1;
}

int CFog::MsgFunc_Fog(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);
	enabled = READ_BYTE();
	color.x = READ_BYTE() / 255.0f;
	color.y = READ_BYTE() / 255.0f;
	color.z = READ_BYTE() / 255.0f;
	startDist = (uint16_t)READ_SHORT();
	endDist = (uint16_t)READ_SHORT();
	return 1;
}

void CFog::SetupFog() {
	if (!enabled)
	{
		gEngfuncs.pTriAPI->Fog(Vector(0, 0, 0), 0, 0, FALSE);
		glDisable(GL_FOG);
		return;
	}

	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, 0.5);

	glFogfv(GL_FOG_COLOR, color);
	glFogf(GL_FOG_START, startDist);
	glFogf(GL_FOG_END, endDist);

	// Tell the engine too
	gEngfuncs.pTriAPI->Fog(color, startDist, endDist, TRUE);
}