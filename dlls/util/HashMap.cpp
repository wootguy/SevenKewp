#include "HashMap.h"
#include <string.h>
#include "extdll.h"
#include "util.h"

#define HMAP_INVALID_STRING 65535
#define HMAP_DEFAULT_MAX_ENTRIES 8
#define HMAP_DEFAULT_STRING_POOL_SZ 64
#define HMAP_MAX_STRING_POOL_SZ 65534 // don't overlap the invalid string value
#define HMAP_MAX_FILL_PERCENT 0.7f // how full the map can be before it's resized

StringMap::StringMap() {
	data = NULL;
	stringOffset = 0;
	maxEntries = 0;
	stringPoolSz = 0;
    memset(&stats, 0, sizeof(hash_map_stats_t));
}

StringMap::StringMap(int maxEntries, uint16_t stringPoolSz) {
    init(maxEntries, stringPoolSz);
}

void StringMap::init(int maxEntries, uint16_t stringPoolSz) {
    stringOffset = 0;
    this->maxEntries = maxEntries;
    this->stringPoolSz = stringPoolSz;

    int dataSz = stringPoolSz + maxEntries * sizeof(string_map_entry_t);

    data = new char[dataSz];
    memset(data, 0, dataSz);
    memset(&stats, 0, sizeof(hash_map_stats_t));

    printStats();
}

void StringMap::clear() {
    delete[] data;
    init(HMAP_DEFAULT_MAX_ENTRIES, HMAP_DEFAULT_STRING_POOL_SZ);
}

StringMap::~StringMap() {
	delete[] data;
}

// FNV1a algorithm
uint32_t StringMap::hash(const char* str) {
    uint64 hash = 14695981039346656037ULL;
    uint32_t c;

    while ((c = *str++)) {
        hash = (hash * 1099511628211) ^ c;
    }

    return hash % maxEntries;
}

// Store string in preallocated memory pool (avoiding malloc)
uint16_t StringMap::storeString(const char* str) {
    size_t len = strlen(str) + 1;  // Include null terminator

    if ((size_t)stringOffset + len >= (size_t)stringPoolSz) {
        size_t needSz = stringPoolSz + len;
        size_t newPoolSz = stringPoolSz * 2;
        while (newPoolSz <= needSz && newPoolSz < HMAP_MAX_STRING_POOL_SZ) {
            newPoolSz *= 2;
        }

        if (!resizeStringPool(newPoolSz)) {
            return HMAP_INVALID_STRING;  // Out of memory
        }
    }

    char* stringPool = data;
    char* stored = stringPool + stringOffset;
    memcpy(stored, str, len);
    stringOffset += len;

    return stored - stringPool;
}

bool StringMap::put(const char* key, const char* value) {
    if (!data) {
        init(HMAP_DEFAULT_MAX_ENTRIES, HMAP_DEFAULT_STRING_POOL_SZ);
    }

    uint32_t index = hash(key);
    uint32_t startIndex = index;
    int oldSz = size();

    int depth = 0;

    uint16_t ival = storeString(value);
    
    char* stringPool = data;
    string_map_entry_t* entries = (string_map_entry_t*)(data + stringPoolSz);

    while (entries[index].occupied) {
        const char* entryKey = stringPool + entries[index].key;

        if (!strcmp(entryKey, key)) {
            entries[index].value = ival; // update existing
            return true;
        }

        index = (index + 1) % maxEntries;   // linear probing

        depth += 1;
        
        if (index == startIndex) {
            ALERT(at_error, "StringMap is full. Failed to insert '%s' = '%s'\n", key, value);
            return false;
        }
    }

    uint16_t ikey = storeString(key);
    stringPool = data;
    entries = (string_map_entry_t*)(data + stringPoolSz);

    // new entry
    string_map_entry_t& newEntry = entries[index];
    newEntry.key = ikey;
    newEntry.value = ival;
    newEntry.occupied = true;

    if (newEntry.key == HMAP_INVALID_STRING || newEntry.value == HMAP_INVALID_STRING) {
        newEntry.occupied = false;
        ALERT(at_error, "StringMap failed to insert '%s' = '%s'\n", key, value);
        return false;
    }

    stats.totalPuts++;
    if (depth > 0) {
        stats.collisions++;
        stats.maxDepth = V_max(stats.maxDepth, depth);
        stats.totalDepth += depth;
    }

    // resize before there's a problem
    if (size() >= reservedSize() * HMAP_MAX_FILL_PERCENT) {
        resizeHashTable(maxEntries * 2);
    }

    return true;
}

bool StringMap::putAll(StringMap& other) {
    if (!other.data) {
        return true;
    }

    char* otherStringPool = other.data;
    string_map_entry_t* otherEntries = (string_map_entry_t*)(other.data + stringPoolSz);

    for (size_t i = 0; i < other.maxEntries; i++) {
        if (otherEntries[i].occupied) {
            if (!put(otherStringPool + otherEntries[i].key, otherStringPool + otherEntries[i].value)) {
                return false;
            }
        }
    }

    return true;
}

const char* StringMap::get(const char* key) {
    if (!data) {
        return NULL;
    }

    char* stringPool = data;
    string_map_entry_t* entries = (string_map_entry_t*)(data + stringPoolSz);
    uint32_t index = hash(key);
    uint32_t startIndex = index;

    while (entries[index].occupied) {
        if (entries[index].occupied && !strcmp(stringPool + entries[index].key, key)) {
            return stringPool + entries[index].value;
        }

        index = (index + 1) % maxEntries;

        if (index == startIndex) {
            return NULL; // table completely full
        }
    }

    return NULL;
}

void StringMap::del(const char* key) {
    if (!data) {
        return;
    }

    char* stringPool = data;
    string_map_entry_t* entries = (string_map_entry_t*)(data + stringPoolSz);
    uint32_t index = hash(key);
    uint32_t startIndex = index;

    while (entries[index].occupied) {
        if (entries[index].occupied && !strcmp(stringPool + entries[index].key, key)) {
            entries[index].occupied = false;  // mark as deleted
            return;
        }

        index = (index + 1) % maxEntries;

        if (index == startIndex) {
            return;
        }
    }
}

int StringMap::size() {
    if (!data) {
        return 0;
    }

    int filled = 0;
    string_map_entry_t* entries = (string_map_entry_t*)(data + stringPoolSz);
    
    for (size_t i = 0; i < maxEntries; i++) {
        filled += entries[i].occupied ? 1 : 0;
    }

    return filled;
}

int StringMap::reservedSize() {
    return maxEntries;
}

bool StringMap::resizeStringPool(size_t newPoolSz) {
    newPoolSz = V_min(HMAP_MAX_STRING_POOL_SZ, newPoolSz);

    if (newPoolSz <= stringPoolSz) {
        ALERT(at_error, "StringMap can't make the string pool any larger!\n");
        return false;
    }

    //ALERT(at_console, "StringMap resized string pool from %d to %d bytes\n", (int)stringPoolSz, (int)newPoolSz);

    char* stringPool = data;
    string_map_entry_t* entries = (string_map_entry_t*)(data + stringPoolSz);

    int newDatSz = newPoolSz + maxEntries * sizeof(string_map_entry_t);
    char* newDat = new char[newDatSz];
    memset(newDat, 0, newDatSz);
    memcpy(newDat, stringPool, stringPoolSz);
    memcpy(newDat + newPoolSz, entries, maxEntries * sizeof(string_map_entry_t));

    delete[] data;
    data = newDat;
    stringPoolSz = newPoolSz;

    return true;
}

bool StringMap::resizeHashTable(size_t newMaxEntries) {    
    if (newMaxEntries <= maxEntries) {
        ALERT(at_error, "StringMap can't make the hash table any larger!\n");
        return false;
    }

    //printStats();
    //ALERT(at_console, "StringMap resized hash table from %d to %d entries\n", (int)maxEntries, (int)newMaxEntries);

    char* oldDat = data;
    char* oldStringPool = data;
    string_map_entry_t* oldEntries = (string_map_entry_t*)(oldDat + stringPoolSz);
    size_t oldMaxEntries = maxEntries;
    int oldSz = size();

    maxEntries = newMaxEntries;
    int dataSz = stringPoolSz + maxEntries * sizeof(string_map_entry_t);
    data = new char[dataSz];
    memset(data, 0, dataSz);
    stringOffset = 0;

    // reset stats so new growth isn't affected by old size stats
    memset(&stats, 0, sizeof(hash_map_stats_t));

    // can't simply memcpy because key hashes will modulo to different slots
    for (size_t i = 0; i < oldMaxEntries; i++) {
        if (!oldEntries[i].occupied) {
            continue;
        }
        if (!put(oldStringPool + oldEntries[i].key, oldStringPool + oldEntries[i].value)) {
            ALERT(at_error, "StringMap failed to put during table resize\n");
        }
    }

    delete[] oldDat;

    //printStats();
}

bool StringMap::iterate(size_t& offset, const char** key, const char** value) {
    char* stringPool = data;
    string_map_entry_t* entries = (string_map_entry_t*)(data + stringPoolSz);

    for (; offset < maxEntries; offset++) {
        if (entries[offset].occupied) {
            *key = stringPool + entries[offset].key;
            *value = stringPool + entries[offset].value;
            offset++;
            return true;
        }
    }

    return false;
}

void StringMap::printStats() {
    int dataSz = stringPoolSz + maxEntries * sizeof(string_map_entry_t) + sizeof(StringMap);
    float avgDepth = stats.totalPuts ? (float)stats.totalDepth / (float)stats.totalPuts : 0.0f;

    ALERT(at_console, "Collisions: %d, Max: %d, Avg: %.2f | Fullness: %d / %d | %d B = %d B entries + %d B strings\n",
        stats.collisions, stats.maxDepth, avgDepth, size(), reservedSize(),
        dataSz, (int)(maxEntries * sizeof(string_map_entry_t)), (int)stringPoolSz);
}