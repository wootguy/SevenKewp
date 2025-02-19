#include "extdll.h"
#include "util.h"
#include "HashMap.h"
#include <string.h>

#define HMAP_DEFAULT_MAX_ENTRIES 8
#define HMAP_DEFAULT_STRING_POOL_SZ 64
#define HMAP_MAX_STRING_POOL_SZ UINT16_MAX

// how full the map can be before it's resized, should be a low percentage because open addressing
// has problems with clustering (collisions in separate "buckets" merging to create giant chains of
// blocked space)
#define HMAP_MAX_FILL_PERCENT 0.5f

BaseHashMap::BaseHashMap(int valueSz) {
    data = NULL;
    stringOffset = 1; // skip 0 for NULL value
    maxEntries = 0;
    stringPoolSz = 0;
    this->entrySz = valueSz + sizeof(hash_map_entry_t);
    memset(&stats, 0, sizeof(hash_map_stats_t));
}

BaseHashMap::BaseHashMap(int valueSz, int maxEntries, uint16_t stringPoolSz) {
    this->entrySz = valueSz + sizeof(hash_map_entry_t);
    init(maxEntries, stringPoolSz);
}

void BaseHashMap::init(int maxEntries, uint16_t stringPoolSz) {
    stringOffset = 1; // skip 0 for NULL value
    this->maxEntries = maxEntries;
    this->stringPoolSz = stringPoolSz;

    int dataSz = stringPoolSz + maxEntries * entrySz;

    data = new char[dataSz];
    memset(data, 0, dataSz);
    memset(&stats, 0, sizeof(hash_map_stats_t));
}

void BaseHashMap::clear() {
    delete[] data;
    init(HMAP_DEFAULT_MAX_ENTRIES, HMAP_DEFAULT_STRING_POOL_SZ);
}

// FNV1a algorithm
uint32_t BaseHashMap::hash(const char* str) const {
    uint64 hash = 14695981039346656037ULL;
    uint32_t c;

    while ((c = *str++)) {
        hash = (hash * 1099511628211) ^ c;
    }

    return hash % maxEntries;
}

BaseHashMap::~BaseHashMap() {
    if (data) {
        delete[] data;
        data = NULL;
    }
}

// Store string in preallocated memory pool (avoiding malloc)
uint16_t BaseHashMap::storeString(const char* str) {
    size_t len = strlen(str) + 1;  // Include null terminator

    if ((size_t)stringOffset + len >= (size_t)stringPoolSz) {
        size_t needSz = stringPoolSz + len;
        size_t newPoolSz = stringPoolSz * 2;
        while (newPoolSz <= needSz && newPoolSz < HMAP_MAX_STRING_POOL_SZ) {
            newPoolSz *= 2;
        }

        if (!resizeStringPool(newPoolSz)) {
            ALERT(at_error, "String pool out of memory!\n");
            return 0;  // out of memory
        }
    }

    uint16_t retOffset = stringOffset;
    char* stringPool = data;
    char* stored = stringPool + stringOffset;
    memcpy(stored, str, len);
    stringOffset += len;

    return retOffset;
}

bool BaseHashMap::put(const char* key, void* value) {
    if (!data) {
        init(HMAP_DEFAULT_MAX_ENTRIES, HMAP_DEFAULT_STRING_POOL_SZ);
    }

    uint32_t index = hash(key);
    uint32_t startIndex = index;
    int oldSz = size();

    int depth = 0;

    char* stringPool = data;
    hash_map_entry_t* entry = (hash_map_entry_t*)(data + stringPoolSz + index*entrySz);
    int valueSz = entrySz - sizeof(hash_map_entry_t);

    while (entry->occupied) {
        const char* entryKey = stringPool + entry->key;

        if (!strcmp(entryKey, key)) {
            memcpy((char*)entry + sizeof(hash_map_entry_t), value, valueSz); // update existing
            return true;
        }

        index = (index + 1) % maxEntries;   // linear probing
        entry = (hash_map_entry_t*)(data + stringPoolSz + index*entrySz);

        depth += 1;

        if (index == startIndex) {
            ALERT(at_error, "StringMap is full. Failed to insert '%s' = '%s'\n", key, value);
            return false;
        }
    }

    uint16_t ikey = storeString(key);
    stringPool = data;
    entry = (hash_map_entry_t*)(data + stringPoolSz + index*entrySz);

    // new entry
    entry->key = ikey;
    entry->occupied = true;
    memcpy((char*)entry + sizeof(hash_map_entry_t), value, valueSz);

    if (!entry->key) {
        entry->occupied = false;
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

bool BaseHashMap::putAll(const BaseHashMap& other) {
    if (!other.data) {
        return true;
    }

    if (entrySz != other.entrySz) {
        ALERT(at_console, "BaseHashMap putAll called with mismatched map types\n");
        return false;
    }

    putAll_internal(other.data, other.maxEntries, other.stringPoolSz);

    return true;
}

void* BaseHashMap::getValue(const char* key) const {
    if (!data) {
        return NULL;
    }

    char* stringPool = data;
    uint32_t index = hash(key);
    uint32_t startIndex = index;
    hash_map_entry_t* entry = (hash_map_entry_t*)(data + stringPoolSz + index * entrySz);

    while (entry->occupied) {
        if (entry->occupied && !strcmp(stringPool + entry->key, key)) {
            return (char*)entry + sizeof(hash_map_entry_t);
        }

        index = (index + 1) % maxEntries;
        entry = (hash_map_entry_t*)(data + stringPoolSz + index * entrySz);

        if (index == startIndex) {
            return NULL; // table completely full
        }
    }

    return NULL;
}

void BaseHashMap::del(const char* key) {
    if (!data) {
        return;
    }

    char* stringPool = data;
    uint32_t index = hash(key);
    uint32_t startIndex = index;
    hash_map_entry_t* entry = (hash_map_entry_t*)(data + stringPoolSz + index * entrySz);

    while (entry->occupied) {
        if (entry->occupied && !strcmp(stringPool + entry->key, key)) {
            // can't actually free the node because lookups are aborted when a free node is found
            //entry->occupied = false;
            entry->key = 0;
            
            // TODO: mark with special flag, then re-hash the table if there are too many deleted nodes
            // don't just wait for the next resize
            return;
        }

        index = (index + 1) % maxEntries;
        entry = (hash_map_entry_t*)(data + stringPoolSz + index * entrySz);

        if (index == startIndex) {
            return;
        }
    }
}

int BaseHashMap::size() {
    if (!data) {
        return 0;
    }

    int filled = 0;

    for (size_t i = 0; i < maxEntries; i++) {
        hash_map_entry_t* entry = (hash_map_entry_t*)(data + stringPoolSz + i * entrySz);
        filled += entry->occupied ? 1 : 0;
    }

    return filled;
}

int BaseHashMap::reservedSize() {
    return maxEntries;
}

bool BaseHashMap::resizeStringPool(size_t newPoolSz) {
    newPoolSz = V_min(HMAP_MAX_STRING_POOL_SZ, newPoolSz);

    if (newPoolSz <= stringPoolSz) {
        ALERT(at_error, "StringMap can't make the string pool any larger!\n");
        return false;
    }

    //ALERT(at_console, "StringMap resized string pool from %d to %d bytes\n", (int)stringPoolSz, (int)newPoolSz);

    char* stringPool = data;
    char* entries = data + stringPoolSz;

    int newDatSz = newPoolSz + maxEntries * entrySz;
    char* newDat = new char[newDatSz];
    memset(newDat, 0, newDatSz);
    memcpy(newDat, stringPool, stringPoolSz);
    memcpy(newDat + newPoolSz, entries, maxEntries * entrySz);

    delete[] data;
    data = newDat;
    stringPoolSz = newPoolSz;

    return true;
}

bool BaseHashMap::resizeHashTable(size_t newMaxEntries) {
    if (newMaxEntries <= maxEntries) {
        ALERT(at_error, "StringMap can't make the hash table any larger!\n");
        return false;
    }

    //printStats();
    //ALERT(at_console, "StringMap resized hash table from %d to %d entries\n", (int)maxEntries, (int)newMaxEntries);

    char* oldDat = data;
    char* oldStringPool = data;
    char* oldEntries = oldDat + stringPoolSz;
    size_t oldMaxEntries = maxEntries;
    int oldSz = size();

    maxEntries = newMaxEntries;
    int dataSz = stringPoolSz + maxEntries * entrySz;
    data = new char[dataSz];
    memset(data, 0, dataSz);
    stringOffset = 1;

    // reset stats so new growth isn't affected by old size stats
    memset(&stats, 0, sizeof(hash_map_stats_t));

    // can't simply memcpy because key hashes will modulo to different slots
    putAll_internal(oldDat, oldMaxEntries, stringPoolSz);

    delete[] oldDat;

    //printStats();
}

void BaseHashMap::printStats() {
    int dataSz = stringPoolSz + maxEntries * entrySz + sizeof(StringMap);
    float avgDepth = stats.totalPuts ? (float)stats.totalDepth / (float)stats.totalPuts : 0.0f;

    ALERT(at_console, "Collisions: %d, Max: %d, Avg: %.2f | Fullness: %d / %d | %.1f KB = %d B entries + %d B strings\n",
        stats.collisions, stats.maxDepth, avgDepth, size(), reservedSize(),
        (float)dataSz / 1024.0f, (int)(maxEntries * entrySz), (int)stringPoolSz);
}

StringMap::StringMap(std::initializer_list<std::pair<const char*, const char*>> init) : BaseHashMap(sizeof(hmap_string_t)) {
    for (const auto& pair : init) {
        put(pair.first, pair.second);
    }
}

bool StringMap::put(const char* key, const char* value) {
    if (!data) {
        init(HMAP_DEFAULT_MAX_ENTRIES, HMAP_DEFAULT_STRING_POOL_SZ);
    }

    uint16_t ival = storeString(value);

    return BaseHashMap::put(key, &ival);
}

void StringMap::putAll_internal(char* otherData, int otherEntryCount, int otherStringPoolSz) {
    for (size_t i = 0; i < otherEntryCount; i++) {
        hash_map_entry_t* entry = (hash_map_entry_t*)(otherData + otherStringPoolSz + i * entrySz);
        if (!entry->occupied) {
            continue;
        }

        uint16_t offset = *(uint16_t*)((char*)entry + sizeof(hash_map_entry_t));
        if (!StringMap::put(otherData + entry->key, otherData + offset)) {
            ALERT(at_error, "StringMap failed to put during table resize\n");
        }
    }
}

const char* StringMap::get(const char* key) const {
    uint16_t* offset = (uint16_t*)getValue(key);
    return offset ? (data + *offset) : NULL;
}

bool StringMap::iterate(size_t& offset, const char** key, const char** value) const {
    char* stringPool = data;

    for (; offset < maxEntries; offset++) {
        hash_map_entry_t* entry = (hash_map_entry_t*)(data + stringPoolSz + offset*entrySz);

        if (entry->occupied) {
            *key = stringPool + entry->key;
            *value = stringPool + *(uint16_t*)((char*)entry + sizeof(hash_map_entry_t));
            offset++;
            return true;
        }
    }

    return false;
}
