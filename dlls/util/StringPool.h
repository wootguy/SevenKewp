#pragma once
#include <stdint.h>
#include <stddef.h>

class StringPool;

// can't simply pass around char* because the string pool can reallocate string buffer when resized
struct mod_string_t {
    StringPool* pool;
    uint32_t offset;

    EXPORT const char* str();
    EXPORT void clear();
};

// allocates strings from a single buffer, using fewer allocations than alternatives like std::string.
// unlike the engine's string_t, these strings can be saved across level changes.
// Can be used as a replacement for std::vector, if you don't need to remove individual elements
class StringPool {
public:
    EXPORT StringPool();
    EXPORT StringPool(uint32_t poolSz);
    EXPORT ~StringPool();
    EXPORT StringPool(const StringPool& other);
    EXPORT StringPool& operator=(const StringPool& other);

    EXPORT void clear();

    // allocate from the pool
    EXPORT mod_string_t alloc(const char* str);

    // return string created by alloc, or NULL for invalid offsets
    EXPORT const char* str(uint32_t offset);

    struct iterator_t {
        size_t offset;
        const char* value;

        iterator_t() : offset(1) {}
    };

    EXPORT bool iterate(iterator_t& iter);

    EXPORT bool hasString(const char* str);

    EXPORT const char* first();

    EXPORT const char* last();

private:
    char* data;
    size_t offset; // next free offset
    uint32_t poolSz;

    void init(uint32_t sz);

    bool resize(uint32_t newSz);

    void copyFrom(const StringPool& other);
};