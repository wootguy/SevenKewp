#pragma once
#include <stdint.h>
#include <stddef.h>

struct string_map_entry_t {
    uint16_t key;       // offset into string pool
    uint16_t value;     // offset into string pool
    bool occupied;
};

struct hash_map_stats_t {
    uint16_t collisions;
    uint16_t maxDepth;
    uint16_t totalDepth;
    uint16_t totalPuts;
};

// Hash map using open addressing and a string pool to avoid excessive memory allocations.
// Keys/value strings can not be used after a put() or class destruction.
class StringMap {
public:
    hash_map_stats_t stats;

    StringMap();
    StringMap(int maxEntries, uint16_t stringPoolSz);
    ~StringMap();

    void clear();

    // add a key to the map
    bool put(const char* key, const char* value);

    // insert all keys from the other map into this one, overwriting any key values that already exist
    bool putAll(StringMap& other);

    // get value by key
    const char* get(const char* key);

    // delete key (TODO: does not clear string memory)
    void del(const char* key);

    // returns number of filled slots
    int size();

    // returns total number of slots in the allocated memory
    int reservedSize();

    // return each entry in the map. pass 0 for first iteration. Returns false at the end of iteration
    bool iterate(size_t& offset, const char** key, const char** value);

    // show map statistics
    void printStats();

private:
    char* data; // allocated memory for both strings and entries
    size_t maxEntries;
    uint16_t stringOffset;    // next free space in string pool
    uint16_t stringPoolSz;

    uint32_t hash(const char* str);

    // store string in string pool. May reallocate data.
    uint16_t storeString(const char* str);

    void init(int maxEntries, uint16_t stringPoolSz);

    bool resizeStringPool(size_t newPoolSz);

    bool resizeHashTable(size_t newMaxEntries);
};