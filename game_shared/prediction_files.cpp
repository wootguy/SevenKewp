#include "prediction_files.h"

// Add any hard-coded file paths used by the client here so that the server can replace them
const char* g_prediction_files[] = {
	// footstep sounds
	"player/pl_step1.wav",
	"player/pl_step3.wav",
	"player/pl_step2.wav",
	"player/pl_step4.wav",

	"player/pl_metal1.wav",
	"player/pl_metal3.wav",
	"player/pl_metal2.wav",
	"player/pl_metal4.wav",

	"player/pl_dirt1.wav",
	"player/pl_dirt3.wav",
	"player/pl_dirt2.wav",
	"player/pl_dirt4.wav",

	"player/pl_duct1.wav",
	"player/pl_duct3.wav",
	"player/pl_duct2.wav",
	"player/pl_duct4.wav",

	"player/pl_grate1.wav",
	"player/pl_grate3.wav",
	"player/pl_grate2.wav",
	"player/pl_grate4.wav",

	"player/pl_tile1.wav",
	"player/pl_tile3.wav",
	"player/pl_tile2.wav",
	"player/pl_tile4.wav",

	"player/pl_slosh1.wav",
	"player/pl_slosh3.wav",
	"player/pl_slosh2.wav",
	"player/pl_slosh4.wav",

	"player/pl_wade1.wav",
	"player/pl_wade3.wav",
	"player/pl_wade2.wav",
	"player/pl_wade4.wav",

	"player/pl_ladder1.wav",
	"player/pl_ladder3.wav",
	"player/pl_ladder2.wav",
	"player/pl_ladder4.wav",

	"player/pl_swim1.wav",
	"player/pl_swim2.wav",
	"player/pl_swim3.wav",
	"player/pl_swim4.wav",

	"player/pl_fallpain3.wav",

	// bullet impacts
	"debris/wood1.wav",
	"debris/wood2.wav",
	"debris/wood3.wav",
	"debris/glass1.wav",
	"debris/glass2.wav",
	"debris/glass3.wav",
	"weapons/bullet_hit1.wav",
	"weapons/bullet_hit2.wav",
	"weapons/ric1.wav",
	"weapons/ric2.wav",
	"weapons/ric3.wav",
	"weapons/ric4.wav",
	"weapons/ric5.wav",

	// weapon sounds
	"weapons/pl_gun3.wav",
	"weapons/dbarrel1.wav",
	"weapons/sbarrel1.wav",
	"weapons/hks1.wav",
	"weapons/hks2.wav",
	"weapons/glauncher.wav",
	"weapons/glauncher2.wav",
	"weapons/357_cock1.wav",
	"weapons/357_shot1.wav",
	"weapons/357_shot2.wav",
	"ambience/pulsemachine.wav",
	"weapons/gauss2.wav",
	"weapons/cbar_miss1.wav",
	"weapons/xbow_fire1.wav",
	"weapons/xbow_reload1.wav",
	"weapons/xbow_hitbod1.wav",
	"weapons/xbow_hitbod2.wav",
	"weapons/xbow_hit1.wav",
	"weapons/rocketfire1.wav",
	"weapons/egon_off1.wav",
	"weapons/egon_run3.wav",
	"weapons/egon_windup2.wav",
	"agrunt/ag_fire1.wav",
	"agrunt/ag_fire2.wav",
	"agrunt/ag_fire3.wav",

	// track train
	"plats/ttrain1.wav",
	"plats/ttrain2.wav",
	"plats/ttrain3.wav",
	"plats/ttrain4.wav",
	"plats/ttrain5.wav",
	"plats/ttrain6.wav",
	"plats/ttrain7.wav",

	// radiation
	"player/geiger1.wav",
	"player/geiger2.wav",
	"player/geiger3.wav",
	"player/geiger4.wav",
	"player/geiger5.wav",
	"player/geiger6.wav",

	// HUD sounds
	"misc/talk.wav",
	"common/wpn_select.wav",
	"common/wpn_hudon.wav",
	"common/wpn_moveselect.wav",
	"common/wpn_hudoff.wav",

	// weapon effects
	"sprites/smoke.spr", 		// gauss beam
	"sprites/hotglow.spr", 		// gauss ball
	"sprites/xbeam1.spr", 		// egon beam
	"sprites/xspark1.spr", 		// egon flare
	"sprites/laserbeam.spr", 	// laser beam (rpg)
	"sprites/laserdot.spr", 	// laser dot (rpg)

	// shells / projectiles
	"models/shell.mdl", 
	"models/shotgunshell.mdl", 
	"models/crossbow_bolt.mdl", 

	// water splashes
	"water/waterblow.wav",
	"water/splash.wav",
	"water/splash2.wav",
	"water/splash3.wav",
	"water/explode3.wav",
	"water/explode5.wav",
	"sprites/splash.spr", 
	"sprites/splash2.spr", 
	"sprites/splashwake.spr", 
};

const int g_prediction_files_sz = sizeof(g_prediction_files) / sizeof(g_prediction_files[0]);

uint8_t g_predMsgData[190];
int g_predMsgLen;
int g_soundvariety;
int g_flashlight_size;

#ifdef CLIENT_DLL
#include "../cl_dll/hud.h"
#include "../cl_dll/cl_util.h"
#include "../cl_dll/engine_pv.h"
#include "parsemsg.h"
#include "mstream.h"
#include "HashMap.h"

StringMap g_predFileRemap;

int __MsgFunc_PredFiles(const char* pszName, int iSize, void* pbuf) {
	g_predFileRemap.clear();

	BEGIN_READ(pbuf, iSize);
	int sz = READ_BYTE();

	static uint8_t dat[256];
	memset(dat, 0, sizeof(dat));

	for (int i = 0; i < sz; i++) {
		dat[i] = READ_BYTE();
	}

	mstream buf((char*)dat, sz);

	for (int i = 0; i < g_prediction_files_sz; i++) {
		if (buf.readBit()) {
			uint16_t idx = buf.readBits(9);

			const char* path = g_prediction_files[i];
			const char* ext = path + strlen(path) - 3;
			bool isModel = strcmp(ext, "wav");
			
			const char* replacement;
			if (isModel) {
				model_t* mdl = gEngfuncs.hudGetModelByIndex(idx);
				
				if (!mdl) {
					PRINTF("Bad prediction replacement model idx %d\n", idx);
					continue;
				}

				replacement = mdl->name;
			}
			else {
				replacement = GetSoundByIndex(idx);
			}

			g_predFileRemap.put(g_prediction_files[i], replacement);
			//PRINTD("%-24s -> %s\n", g_prediction_files[i], replacement);
		}
	}

	return 1;
}

int __MsgFunc_PredCvars(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	g_soundvariety = READ_BYTE();
	g_flashlight_size = READ_BYTE();

	return 1;
}

void HookPredictionMessages() {
	HOOK_MESSAGE(PredFiles);
	HOOK_MESSAGE(PredCvars);
}

const char* RemapFile(const char* path) {
	const char* remap = g_predFileRemap.get(path);
	return remap ? remap : path;
}

#endif

#ifndef CLIENT_DLL

#include "extdll.h"
#include "util.h"

int PredRemap(const char* path, bool isModel = false) {
	const char* replacement = isModel ? g_modelReplacements.get(path) 
		: UTIL_GetReplacementSound(NULL, path);

	if (!replacement || !strcmp(path, replacement)) {
		return -1; // not replaced, no need to send
	}

	if (isModel) {
		return g_precachedModels.hasKey(replacement) ? g_engfuncs.pfnModelIndex(replacement) : -1;
	}

	int* precache = g_precachedSounds.get(replacement);

	return precache ? *precache : -1; // index of the replacement
}

void GeneratePredicionData() {
	memset(g_predMsgData, 0, sizeof(g_predMsgData));
	g_predMsgLen = 0;
	mstream buf((char*)g_predMsgData, sizeof(g_predMsgData));

	for (int i = 0; i < g_prediction_files_sz; i++) {

		const char* path = g_prediction_files[i];
		const char* ext = path + strlen(path) - 3;
		bool isModel = strcmp(ext, "wav");
		int remap = PredRemap(path, isModel);

		if (remap != -1) {
			buf.writeBit(1);
			buf.writeBits(remap, 9);
		}
		else {
			buf.writeBit(0);
		}
	}

	buf.endBitWriting();
	const char* suff = buf.eom() ? " (OVERFLOW!!!)" : "";

	g_predMsgLen = buf.tell();
	
	ALERT(buf.eom() ? at_error : at_console, "Prediction file replacement data is %d bytes%s\n", g_predMsgLen + 1, suff);
}

#endif