#include "Scheduler.h"
#include "util.h"
#include "perf.h"
#include <type_traits>

using namespace std;

Scheduler g_Scheduler;
unsigned int g_schedule_id = 1;

void Scheduler::Think() {
    float now = g_engfuncs.pfnTime();

    struct NamedFunc {
        function<void()> func;
        string plugin;
        string name;
    };

    vector<NamedFunc> funcsToCall;

    for (int i = 0; i < (int)functions.size(); i++) {
        ScheduledFunction_internal& func = functions[i];

        if (now - func.lastCall < func.delay) {
            continue;
        }

        // wait to call function in case it adds/removes schedules and messes up this loop
        NamedFunc namedFunc;
        namedFunc.func = func.func;
        namedFunc.name = func.funcName;
        namedFunc.plugin = func.owner;
        funcsToCall.push_back(namedFunc);
        
        func.lastCall = now;
        func.callCount++;

        if (func.maxCalls >= 0 && func.callCount >= func.maxCalls) {
            functions.erase(functions.begin() + i);
            i--;
        }
    }

    for (int i = 0; i < (int)funcsToCall.size(); i++) {
        NamedFunc& func = funcsToCall[i];
        uint64_t now = getEpochMillis();

        func.func();

        perf_log_plugin_hook_timing(func.plugin.c_str(), func.name.c_str(), getEpochMillis() - now);
    }
}

void Scheduler::RemoveTimer(ScheduledFunction sched) {
    for (int i = 0; i < (int)functions.size(); i++) {
        if (functions[i].scheduleId == sched.scheduleId) {
            functions.erase(functions.begin() + i);
            return;
        }
    }
}

void Scheduler::RemoveTimers(const char* owner) {
    std::vector<ScheduledFunction_internal> newFuncs;

    for (int i = 0; i < (int)functions.size(); i++) {
        if (strcmp(functions[i].owner, owner)) {
            newFuncs.push_back(functions[i]);
        }
    }

    functions = newFuncs;
}

bool ScheduledFunction::HasBeenRemoved() {
    for (int i = 0; i < (int)g_Scheduler.functions.size(); i++) {
        if (g_Scheduler.functions[i].scheduleId == scheduleId) {
            return false;
        }
    }
    return true;
}