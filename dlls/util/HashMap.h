#pragma once
#include <stdint.h>
#include <stddef.h>

typedef uint16_t hmap_string_t;

struct hash_map_stats_t {
    uint16_t collisions;
    uint16_t maxDepth;
    uint16_t totalDepth;
    uint16_t totalPuts;
};

struct hash_map_entry_t {
    bool occupied;
    hmap_string_t key; // offset into string pool
    // value follows the header, which could be any size
};

class BaseHashMap {
public:
    hash_map_stats_t stats;

    BaseHashMap(int valueSz);
    BaseHashMap(int valueSz, int maxEntries, uint16_t stringPoolSz);
    ~BaseHashMap();

    void clear();

    // returns number of filled slots
    int size();

    // returns total number of slots in the allocated memory
    int reservedSize();

    // show map statistics
    void printStats();

    // add a key to the map
    bool put(const char* key, void* value);

    // insert all keys from the other map into this one, overwriting any key values that already exist
    bool putAll(const BaseHashMap& other);

    // delete key (TODO: does not clear string memory)
    void del(const char* key);

protected:
    char* data; // allocated memory for both strings and entries
    size_t maxEntries;
    uint16_t stringOffset;    // next free space in string pool
    uint16_t stringPoolSz;
    int entrySz;

    uint32_t hash(const char* str);

    // store string in string pool. May reallocate data.
    uint16_t storeString(const char* str);

    void init(int maxEntries, uint16_t stringPoolSz);

    bool resizeStringPool(size_t newPoolSz);

    bool resizeHashTable(size_t newMaxEntries);

    void* getValue(const char* key);

    virtual void putAll_internal(char* otherData, int otherEntryCount, int otherStringPoolSz) = 0;
};

// Hash map using open addressing and a string pool to avoid excessive memory allocations.
// Keys/value strings can not be used after a put() or class destruction.
// StringMap allocates both keys and values using the same string pool
class StringMap : public BaseHashMap {
public:
    StringMap() : BaseHashMap(sizeof(hmap_string_t)) {}
    StringMap(int maxEntries, uint16_t stringPoolSz) : BaseHashMap(sizeof(hmap_string_t), maxEntries, stringPoolSz) {}

    // add a key to the map
    bool put(const char* key, const char* value);

    // get value by key
    const char* get(const char* key);

    // return each entry in the map. pass 0 for first iteration. Returns false at the end of iteration
    bool iterate(size_t& offset, const char** key, const char** value);

private:
    void putAll_internal(char* otherData, int otherEntryCount, int otherStringPoolSz) override;
};

// Hash map using open addressing and a string pool to avoid excessive memory allocations.
// Templatized to take any POD as values (not anything complex like std::string or std::vector)
template <typename T>
class HashMap : public BaseHashMap {
public:
    hash_map_stats_t stats;

    HashMap() : BaseHashMap(sizeof(T)) {}
    HashMap(int maxEntries, uint16_t stringPoolSz) : BaseHashMap(sizeof(T), maxEntries, stringPoolSz) {}

    bool put(const char* key, const T& value) {
        return BaseHashMap::put(key, (void*)&value);
    }

    T* get(const char* key) {
        return (T*)getValue(key);
    }

    bool iterate(size_t& offset, const char** key, T** value) {
        char* stringPool = data;

        for (; offset < maxEntries; offset++) {
            hash_map_entry_t* entry = (hash_map_entry_t*)(data + stringPoolSz + offset * entrySz);

            if (entry->occupied) {
                *key = stringPool + entry->key;
                *value = (T*)((char*)entry + sizeof(hash_map_entry_t));
                offset++;
                return true;
            }
        }

        return false;
    }

private:
    void putAll_internal(char* otherData, int otherEntryCount, int otherStringPoolSz) override {
        for (size_t i = 0; i < otherEntryCount; i++) {
            hash_map_entry_t* entry = (hash_map_entry_t*)(otherData + otherStringPoolSz + i * entrySz);
            if (!entry->occupied) {
                continue;
            }

            if (!BaseHashMap::put(otherData + entry->key, (char*)entry + sizeof(hash_map_entry_t))) {
                //ALERT(at_error, "StringMap failed to put during table resize\n");
            }
        }
    }
};