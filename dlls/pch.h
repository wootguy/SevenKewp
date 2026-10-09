#include "extdll.h"
#include "util.h"
#include "perf.h"

#include "CBasePlayer.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_set>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef min
#undef max
#undef RGB
#undef GetMessage
#endif