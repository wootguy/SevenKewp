#define SEVENKEWP_VERSION_MAJOR 0 // Set to 1 on release, then never touch again
#define SEVENKEWP_VERSION_MINOR 1 // Bump when there is a backwards incompatible change (e.g. new/changed network messages)
#define SEVENKEWP_VERSION_PATCH 0 // Bump when there is an optional update which doesn't affect the API

#define SEVENKEWP_VERSION \
((SEVENKEWP_VERSION_MAJOR*10000*10000) + (SEVENKEWP_VERSION_MINOR * 10000) + SEVENKEWP_VERSION_PATCH)
