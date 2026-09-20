#include "rehlds.h"
#include "util.h"
#include "perf.h"
#include "CBaseEntity.h"
#include <unordered_set>

using namespace std;

PerfMetrics g_perf_metrics;
unordered_map<string, uint32_t> g_ent_class_timings;
unordered_map<string, uint32_t> g_plugin_hook_timings;
deque<PerfWarning> g_perf_warnings;

PerfWarning::PerfWarning(std::string source, std::string func, uint32_t millis)
	: source(source), func(func), millis(millis), logTime(getEpochMillis()) {}

void perf_init() {
	if (!mp_perf.value)
		return;

	g_ent_class_timings.clear();
	g_plugin_hook_timings.clear();
	memset(&g_perf_metrics, 0, sizeof(g_perf_metrics));
}

void perf_finalize() {
	if (!mp_perf.value)
		return;

	perftimings_t* timings = g_RehldsData->GetPerfTimings();
	if (timings) {
		g_perf_metrics.frame = timings->frame;
		g_perf_metrics.frameInt = timings->frameInt;
		g_perf_metrics.entityPhysics = g_perf_metrics.entityThinks < timings->physics ? 
			(timings->physics - g_perf_metrics.entityThinks) : 0;
		g_perf_metrics.readPackets = timings->readPackets;
		g_perf_metrics.addToFullPack = timings->addToFullPack;
		g_perf_metrics.sendClientMessages = timings->addToFullPack < timings->sendClientMessages ? 
			(timings->sendClientMessages - timings->addToFullPack) : 0;
	}

	g_perf_metrics.playerThink = g_perf_metrics.playerPostThink + g_perf_metrics.playerPreThink;

	if (g_perf_metrics.entityPhysics >= mp_perf.value)
		perf_add_warning("GAME", "EntityPhysics", g_perf_metrics.entityPhysics);
	if (g_perf_metrics.entityThinks >= mp_perf.value)
		perf_add_warning("GAME", "EntityThinks", g_perf_metrics.entityThinks);

	if (g_perf_metrics.playerThink >= mp_perf.value)
		perf_add_warning("GAME", "PlayerThinks", g_perf_metrics.playerThink);
	if (g_perf_metrics.playerPreThink >= mp_perf.value)
		perf_add_warning("GAME", "PlayerPreThinks", g_perf_metrics.playerPreThink);
	if (g_perf_metrics.playerPostThink >= mp_perf.value)
		perf_add_warning("GAME", "PlayerPostThinks", g_perf_metrics.playerPostThink);

	if (g_perf_metrics.readPackets >= mp_perf.value)
		perf_add_warning("GAME", "SV_ReadPackets", g_perf_metrics.readPackets);

	if (g_perf_metrics.sendClientMessages >= mp_perf.value)
		perf_add_warning("GAME", "SV_SendClientMessages", g_perf_metrics.sendClientMessages);
	if (g_perf_metrics.addToFullPack >= mp_perf.value)
		perf_add_warning("GAME", "AddToFullPack", g_perf_metrics.addToFullPack);
}

void perf_add_warning(std::string source, std::string func, uint32_t millis) {
	g_perf_warnings.push_front(PerfWarning(source, func, millis));

	if (g_perf_warnings.size() > 100) {
		g_perf_warnings.pop_back();
	}
}

void perf_log_ent_timing(CBaseEntity* ent, uint32_t millis) {
	if (!millis || !mp_perf.value)
		return;

	if (millis >= mp_perf.value) {
		perf_add_warning(UTIL_VarArgs("'%s'", STRING(ent->pev->targetname)), STRING(ent->pev->classname), millis);
		ALERT(at_warning, "'%s' (%s) Think took %d ms\n", STRING(ent->pev->targetname), STRING(ent->pev->classname), millis);
	}

	g_ent_class_timings[STRING(ent->pev->classname)] += millis;
	g_perf_metrics.entityThinks += millis;
}

void perf_log_plugin_hook_timing(const char* plugin, const char* hook, uint32_t millis) {
	if (!millis || !mp_perf.value)
		return;

	if (millis >= mp_perf.value) {
		perf_add_warning(plugin, hook, millis);
		ALERT(at_warning, "[%s] %s took %d ms\n", plugin, hook, millis);
	}

	g_plugin_hook_timings[string(plugin) + hook] += millis;
}
