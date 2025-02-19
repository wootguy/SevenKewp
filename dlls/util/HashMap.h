#pragma once
#include <stdint.h>
#include <stddef.h>
#include <initializer_list>
#include <utility>

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

    EXPORT BaseHashMap(int valueSz);
    EXPORT BaseHashMap(int valueSz, int maxEntries, uint16_t stringPoolSz);
    EXPORT ~BaseHashMap();

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

    uint32_t hash(const char* str) const;

    // store string in string pool. May reallocate data.
    uint16_t storeString(const char* str);

    void init(int maxEntries, uint16_t stringPoolSz);

    bool resizeStringPool(size_t newPoolSz);

    bool resizeHashTable(size_t newMaxEntries);

    void* getValue(const char* key) const;

    virtual void putAll_internal(char* otherData, int otherEntryCount, int otherStringPoolSz) = 0;
};

// Maps strings to other strings.
// open addressing and a string pool is used to avoid excessive memory allocations.
class StringMap : public BaseHashMap {
public:
    StringMap() : BaseHashMap(sizeof(hmap_string_t)) {}
    StringMap(int maxEntries, uint16_t stringPoolSz) : BaseHashMap(sizeof(hmap_string_t), maxEntries, stringPoolSz) {}
    EXPORT StringMap(std::initializer_list<std::pair<const char*, const char*>> init);

    // add a key to the map
    bool put(const char* key, const char* value);

    // get value by key
    const char* get(const char* key) const;

    // return each entry in the map. pass 0 for first iteration. Returns false at the end of iteration
    bool iterate(size_t& offset, const char** key, const char** value) const;

private:
    EXPORT void putAll_internal(char* otherData, int otherEntryCount, int otherStringPoolSz) override;
};

// Maps a string to a POD type (not anything with a constructor like std::string or std::vector)
// open addressing and a string pool is used to avoid excessive memory allocations.
template <typename T>
class HashMap : public BaseHashMap {
public:
    hash_map_stats_t stats;

    HashMap() : BaseHashMap(sizeof(T)) {}
    HashMap(int maxEntries, uint16_t stringPoolSz) : BaseHashMap(sizeof(T), maxEntries, stringPoolSz) {}
    HashMap(std::initializer_list<std::pair<const char*, T>> init) : BaseHashMap(sizeof(T)) {
        for (const auto& pair : init) {
            put(pair.first, pair.second);
        }
    }

    bool put(const char* key, const T& value) {
        return BaseHashMap::put(key, (void*)&value);
    }

    T* get(const char* key) const {
        return (T*)getValue(key);
    }

    bool iterate(size_t& offset, const char** key, T** value) const {
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
