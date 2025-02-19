#include "StringPool.h"
#include <string.h>
#include "extdll.h"
#include "util.h"

#define DEFAULT_STRING_POOL_SIZE 256
#define MAX_STRING_POOL_SIZE 1024*1024*8 // prevent insane size growth/leaks

StringPool::StringPool() {
	init(DEFAULT_STRING_POOL_SIZE);
}

StringPool::StringPool(uint32_t poolSz) {
	init(poolSz);
}

StringPool::~StringPool() {
    delete[] data;
}

void StringPool::init(uint32_t sz) {
	offset = 1; // skip 0 offset so that 0 can be used for invalid strings
	poolSz = sz;
	data = new char[sz];
    memset(data, 0, sz);
}

void StringPool::clear() {
    delete[] data;
    init(DEFAULT_STRING_POOL_SIZE);
}

mod_string_t StringPool::alloc(const char* str) {
    size_t len = strlen(str) + 1;  // Include null terminator

    mod_string_t ret;
    ret.pool = this;
    ret.offset = 0;

    if ((size_t)offset + len >= (size_t)poolSz) {
        size_t needSz = poolSz + len;
        size_t newPoolSz = poolSz * 2;
        while (newPoolSz <= needSz && newPoolSz < MAX_STRING_POOL_SIZE) {
            newPoolSz *= 2;
        }

        if (!resize(newPoolSz)) {
            return ret; // out of memory
        }
    }

    char* stringPool = data;
    char* stored = stringPool + offset;
    memcpy(stored, str, len);
    offset += len;

    ret.offset = stored - stringPool;
    return ret;
}

const char* StringPool::str(uint32_t offset) {
    return (offset && offset < poolSz) ? (data + offset) : NULL;
}

const char* mod_string_t::str() {
    return pool ? pool->str(offset) : NULL;
}

void mod_string_t::clear() {
    pool = NULL;
    offset = 0;
}

bool StringPool::resize(uint32_t newPoolSz) {
    newPoolSz = V_min(MAX_STRING_POOL_SIZE, newPoolSz);

    if (newPoolSz <= poolSz) {
        ALERT(at_error, "StringPool can't grow any larger!\n");
        return false;
    }

    //ALERT(at_console, "StringPool resized from %d to %d bytes\n", (int)poolSz, (int)newPoolSz);

    char* newDat = new char[newPoolSz];
    memset(newDat, 0, newPoolSz);
    memcpy(newDat, data, poolSz);

    delete[] data;
    data = newDat;
    poolSz = newPoolSz;

    return true;
}