#include "shared_util.h"

char* strcpy_safe(char* dest, const char* src, size_t size) {
	if (size > 0) {
		size_t i;
		for (i = 0; i < size - 1 && src[i]; i++) {
			dest[i] = src[i];
		}
		dest[i] = '\0';
	}
	return dest;
}

char* strcat_safe(char* dest, const char* src, size_t size) {
	size_t dstLen = strlen(dest);
	char* writeDst = dest + dstLen;
	
	if (dstLen > size) {
		return dest;
	}
	size -= dstLen;

	if (size > 0) {
		size_t i;
		for (i = 0; i < size - 1 && src[i]; i++) {
			writeDst[i] = src[i];
		}
		writeDst[i] = '\0';
	}
	return dest;
}
