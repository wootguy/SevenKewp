/*
* util functions that both the serverand client need
*/
#pragma once
#include "Platform.h"
#include <string.h>

// same as strncpy except it ensures the destination is null terminated, even if the buffer is too small
EXPORT char* strcpy_safe(char* dest, const char* src, size_t size);

// same as strncat except it ensures the destination is null terminated, even if the buffer is too small
EXPORT char* strcat_safe(char* dest, const char* src, size_t size);

// hash client data files for auto-updates in bulk
const char* UTIL_HashClientDataFiles();

// called by the client only to delete sevenkewp files to prepare for an update
void UTIL_DeleteClientDataFiles();

EXPORT float normalizeRangef(const float value, const float start, const float end);