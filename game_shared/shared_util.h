/*
* util functions that both the serverand client need
*/
#pragma once
#include <string.h>

// same as strncpy except it ensures the destination is null terminated, even if the buffer is too small
char* strcpy_safe(char* dest, const char* src, size_t size);

// same as strncat except it ensures the destination is null terminated, even if the buffer is too small
char* strcat_safe(char* dest, const char* src, size_t size);
