#pragma once
#include <stdint.h>

// private engine data
struct EnginePv {
	int32_t* r_wpoly; // rendered world poly count
	int32_t* r_epoly; // rendered studio model poly count
};

extern EnginePv g_enginepv;

// scan process memory to find engine data pointers
void InitEnginePv();