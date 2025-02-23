#pragma once
#include <stdint.h>
#include <stddef.h>
#include <initializer_list>
#include <utility>
#include <unordered_map>
#include <vector>
#include "shared_util.h"

typedef uint16_t hmap_string_t; // offset into a hashmap's string pool

struct hash_map_stats_t {
	uint16_t collisions;
	uint16_t maxDepth;
	uint16_t totalDepth;
};

class BaseHashMap {
public:
	struct entry_header_t {
		bool occupied;
		hmap_string_t key;
		// value follows this header, which could be any size
	};

	hash_map_stats_t stats;

	EXPORT BaseHashMap(int valueSz);
	EXPORT BaseHashMap(int valueSz, int maxEntries, uint16_t stringPoolSz);
	EXPORT ~BaseHashMap();
	EXPORT BaseHashMap(const BaseHashMap& other);
	EXPORT BaseHashMap& operator=(const BaseHashMap& other);

	EXPORT void clear();

	// returns number of filled slots
	EXPORT int size();

	// returns total number of slots in the allocated memory
	EXPORT int reservedSize();

	// show map statistics
	EXPORT void printStats();

	// add a key to the map
	EXPORT bool put(const char* key, void* value);

	// insert all keys from the other map into this one, overwriting any key values that already exist
	EXPORT bool putAll(const BaseHashMap& other);

	// delete key (TODO: does not clear string memory)
	EXPORT void del(const char* key);

	// for debugging
	EXPORT std::vector<std::pair<std::string, std::string>> print();

	EXPORT static void global_hash_stats();

protected:
	char* data; // allocated memory for both strings and entries
	size_t maxEntries;
	size_t entryCount;
	uint16_t stringOffset;    // next free space in string pool
	uint16_t stringPoolSz;
	int entrySz;

	uint32_t hash(const char* str) const;

	// store string in string pool. May reallocate data.
	uint16_t storeString(const char* str);

	// returns:
	// - true if the value was found. index points to table index
	// - false if not found. index points to free space for insertion
	// depth is the amount of table elements that needed to be checked before finding/not finding the key
	bool find(const char* key, uint32_t& index, uint32_t& depth) const;

	void init(int maxEntries, uint16_t stringPoolSz);

	bool resizeStringPool(size_t newPoolSz);

	bool resizeHashTable(size_t newMaxEntries);

	void copyFrom(const BaseHashMap& other);

	EXPORT void* getValue(const char* key) const;

	virtual const char* getValueString(void* value) = 0;

	virtual void putAll_internal(char* otherData, size_t otherEntryCount, size_t otherStringPoolSz) = 0;
};

// Maps strings to other strings.
// open addressing and a string pool is used to avoid excessive memory allocations.
class StringMap : public BaseHashMap {
public:
	StringMap() : BaseHashMap(sizeof(hmap_string_t)) {}
	StringMap(int maxEntries, uint16_t stringPoolSz) : BaseHashMap(sizeof(hmap_string_t), maxEntries, stringPoolSz) {}
	EXPORT StringMap(std::initializer_list<std::pair<const char*, const char*>> init);

	// add a key to the map
	EXPORT bool put(const char* key, const char* value);

	// get value by key
	EXPORT const char* get(const char* key) const;

	struct iterator_t {
		size_t offset;
		const char* key;
		const char* value;

		iterator_t() : offset(0) {}
	};

	EXPORT bool iterate(iterator_t& iter) const;

private:
	EXPORT void putAll_internal(char* otherData, size_t otherEntryCount, size_t otherStringPoolSz) override;

	EXPORT const char* getValueString(void* value) override;
};

// Hash table for strings
class StringSet : public BaseHashMap {
public:
	StringSet() : BaseHashMap(0) {}
	StringSet(int maxEntries, uint16_t stringPoolSz) : BaseHashMap(0, maxEntries, stringPoolSz) {}
	EXPORT StringSet(std::initializer_list<const char*> init);

	// add a key to the map
	EXPORT bool put(const char* key);

	// returns true if the key exists in the set
	EXPORT bool hasKey(const char* key) const;

	struct iterator_t {
		size_t offset;
		const char* key;

		iterator_t() : offset(0) {}
	};

	// return each entry in the map. pass 0 for first iteration. Returns false at the end of iteration
	EXPORT bool iterate(iterator_t& iter) const;

private:
	EXPORT void putAll_internal(char* otherData, size_t otherEntryCount, size_t otherStringPoolSz) override;

	EXPORT const char* getValueString(void* value) override { return ""; };
};

EXPORT char* UTIL_VarArgs(const char* format, ...);

// Maps a string to a POD type (not anything with a constructor like std::string or std::vector)
// open addressing and a string pool is used to avoid excessive memory allocations.
template <typename T>
class HashMap : public BaseHashMap {
public:
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

	struct iterator_t {
		size_t offset;
		const char* key;
		T* value;

		iterator_t() : offset(0) {}
	};

	bool iterate(iterator_t& iter) const {
		char* stringPool = data;

		for (; iter.offset < maxEntries; iter.offset++) {
			entry_header_t* entry = (entry_header_t*)(data + stringPoolSz + iter.offset * entrySz);

			if (entry->occupied) {
				iter.key = stringPool + entry->key;
				iter.value = (T*)((char*)entry + sizeof(entry_header_t));
				iter.offset++;
				return true;
			}
		}

		return false;
	}

private:
	void putAll_internal(char* otherData, size_t otherEntryCount, size_t otherStringPoolSz) override {
		for (size_t i = 0; i < otherEntryCount; i++) {
			entry_header_t* entry = (entry_header_t*)(otherData + otherStringPoolSz + i * entrySz);
			if (!entry->occupied) {
				continue;
			}

			if (!BaseHashMap::put(otherData + entry->key, (char*)entry + sizeof(entry_header_t))) {
				//ALERT(at_error, "StringMap failed to put during table resize\n");
			}
		}
	}

	const char* str(void* value) {
		// treat as bytes if there are no specialized overloads
		const int max_value_len = 64;
		static char output[max_value_len];
		int valueSz = entrySz - sizeof(entry_header_t);

		output[0] = 0;
		strcat_safe(output, "0x", max_value_len);

		uint8_t* bytes = (uint8_t*)value;
		for (int i = valueSz - 1; i >= 0; i--) {
			strcat_safe(output, UTIL_VarArgs("%02X", (int)bytes[i]), max_value_len);
		}
		if (strlen(output) >= max_value_len - 1) {
			output[max_value_len - 16] = 0;
			strcat_safe(output, UTIL_VarArgs("... (%d bytes)", valueSz), max_value_len);
		}

		return output;
	}

	const char* getValueString(void* value) override {
		return str(value);
	}
};

template <> EXPORT const char* HashMap<bool>::str(void* value);
template <> EXPORT const char* HashMap<char>::str(void* value);
template <> EXPORT const char* HashMap<short>::str(void* value);
template <> EXPORT const char* HashMap<int>::str(void* value);
template <> EXPORT const char* HashMap<long>::str(void* value);
template <> EXPORT const char* HashMap<uint8_t>::str(void* value);
template <> EXPORT const char* HashMap<uint16_t>::str(void* value);
template <> EXPORT const char* HashMap<uint32_t>::str(void* value);
template <> EXPORT const char* HashMap<unsigned long>::str(void* value);
template <> EXPORT const char* HashMap<int64_t>::str(void* value);
template <> EXPORT const char* HashMap<uint64_t>::str(void* value);
template <> EXPORT const char* HashMap<float>::str(void* value);
template <> EXPORT const char* HashMap<double>::str(void* value);
template <> EXPORT const char* HashMap<long double>::str(void* value);

EXPORT extern const StringMap g_emptyStringMap;