/*
* util functions that both the serverand client need
*/
#pragma once
#include <string.h>

char* strcpy_safe(char* dest, const char* src, size_t size);

char* strcat_safe(char* dest, const char* src, size_t size);
