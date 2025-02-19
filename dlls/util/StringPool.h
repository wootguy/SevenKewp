#pragma once
#include <stdint.h>
#include <stddef.h>

class StringPool;

// can't simply pass around char* because the string pool can reallocate string buffer when resized
struct mod_string_t {
    StringPool* pool;
    uint32_t offset;

    const char* str();
};

// allocates strings from a single buffer, using fewer allocations than alternatives like std::string.
// unlike the engine's string_t, these strings can be saved across level changes
class StringPool {
public:
	StringPool();
	StringPool(uint32_t poolSz);
	~StringPool();

    void clear();

    // allocate from the pool
    mod_string_t alloc(const char* str);

    // return string created by alloc, or NULL for invalid offsets
    const char* str(uint32_t offset);

private:
    char* data;
    size_t offset; // next free offset
    uint32_t poolSz;

    void init(uint32_t sz);

    bool resize(uint32_t newSz);
};