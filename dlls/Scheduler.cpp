#include "Scheduler.h"
#include "util.h"
#include <type_traits>

using namespace std;

Scheduler g_Scheduler;
unsigned int g_schedule_id = 1;

void Scheduler::Think() {
    float now = g_engfuncs.pfnTime();

    vector<function<void()>> funcsToCall;

    for (int i = 0; i < (int)functions.size(); i++) {
        ScheduledFunction_internal& func = functions[i];

        if (now - func.lastCall < func.delay) {
            continue;
        }

        // wait to call function in case it adds/removes schedules and messes up this loop
        funcsToCall.push_back(func.func);
        
        func.lastCall = now;
        func.callCount++;

        if (func.maxCalls >= 0 && func.callCount >= func.maxCalls) {
            functions.erase(functions.begin() + i);
            i--;
        }
    }

    for (int i = 0; i < (int)funcsToCall.size(); i++) {
        funcsToCall[i]();
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