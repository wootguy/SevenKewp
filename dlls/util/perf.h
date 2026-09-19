#pragma once
#include <unordered_map>
#include <string>
#include <deque>
#include <stdint.h>

class CBaseEntity;

void perf_init();

void perf_finalize();

void perf_log_ent_timing(CBaseEntity* ent, uint32_t millis);

void perf_log_plugin_hook_timing(const char* plugin, const char* hook, uint32_t millis);

struct PerfMetrics {
	// engine timings
	uint32_t frame;					// overall frame time, including idle for ticrate
	uint32_t frameInt;				// internal frame time
	uint32_t readPackets;			// millis spent reading packets and running player thinks/moves
	uint32_t sendClientMessages;	// millis spent transmitting messages
	uint32_t addToFullPack;			// millis spent in AddToFullPack
	uint32_t entityPhysics;			// time spent moving entities, includes Blocked hooks

	// mod timings
	uint32_t entityThinks;			// all entity think timings summed
	uint32_t playerThink;
	uint32_t playerPreThink;
	uint32_t playerPostThink;
	uint32_t playerMove;
	uint32_t pluginFuncs;			// total time spent in timed plugin hooks and timed functions
};

struct PerfWarning {
	std::string source;
	std::string func;
	uint32_t millis;
	uint64_t logTime;

	PerfWarning() {}
	PerfWarning(std::string source, std::string func, uint32_t millis);
};

EXPORT extern PerfMetrics g_perf_metrics;
EXPORT extern std::unordered_map<std::string, uint32_t> g_ent_class_timings;	// think timings per entity class
EXPORT extern std::unordered_map<std::string, uint32_t> g_plugin_hook_timings;	// timings per plugin hook
EXPORT extern std::deque<PerfWarning> g_perf_warnings;							// timings per plugin hook