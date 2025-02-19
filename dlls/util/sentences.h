#pragma once
#include "extdll.h"
#include "EHandle.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include "HashMap.h"
#include "StringPool.h"

class CBaseToggle;

#define MAX_SENTENCE_WORDS 16

struct SentencePart {
	mod_string_t file; // path to the sound file
	uint8_t volume; // 0-100
	uint8_t pitch; // 0-255
	uint8_t start; // 0-100 (can't implement for vanilla hl clients)
	uint8_t end; // 0-100
	uint8_t tempo; // 0-100 (can't implement for vanilla hl clients)
	uint16_t duration; // precalculated playback time for the sound (milliseconds)
};

struct CustomSentence {
	mod_string_t name; // name, excluding the number suffix
	SentencePart words[MAX_SENTENCE_WORDS];
	uint8_t numWords;
};

struct CustomSentenceMap {
	HashMap<CustomSentence> map;
	StringPool strings; // string pool which sentence values allocate from

	void clear() {
		map.clear();
		strings.clear();
	}
};

// maps a sentence name ("SC_OK1") to a a custom sentence 
extern CustomSentenceMap g_customSentencesMod;
extern CustomSentenceMap g_customSentencesMap;
extern CustomSentenceMap g_customSentences; // combined mod and map sentences

// maps a sentence group name ("SC_OK") to a list of custom sentence names ("SC_OK0", "SC_OK1", "SC_OK2")
extern std::unordered_map<std::string, std::vector<std::string>> g_customSentenceGroupsMod;
extern std::unordered_map<std::string, std::vector<std::string>> g_customSentenceGroupsMap;
extern std::unordered_map<std::string, std::vector<std::string>> g_customSentenceGroups; // combined mod and map sentence groups

// do not call this while the map is running or else entity sentence pointers are invalidated
void LoadSentenceFile(const char* path,
	CustomSentenceMap& sentences,
	std::unordered_map<std::string, std::vector<std::string>>& sentenceGroups);

// parses a line from sentences.txt (example: "WILD5 ambience/(v50 p97) quail1, quail1(t15)")
EXPORT CustomSentence ParseSentence(StringPool& stringPool, std::string sentenceLine);

// get custom sentence by exact name
// pointer is invalidated if sentence maps are touched
EXPORT CustomSentence* GetCustomSentence(std::string sentenceName);

// get a random custom sentence by group name
// pointer is invalidated if sentence maps are touched
EXPORT CustomSentence* GetRandomCustomSentence(std::string sentenceGroupName);

// calculates and sets the duration field
EXPORT void CalculateAndSetPlaybackTime(SentencePart& word);

// begins playback of a custom sentence on the given entity
EXPORT void AddCustomSentencePlayer(CBaseToggle* ent, CustomSentence* sent, float volume, float attn);

// name = specific sentence ("!SC_OK1") or a group name ("SC_OK")
EXPORT bool PrecacheCustomSentence(CBaseEntity* ent, std::string name);

// begins or continues playback of a custom sentence. called by PlayCustomSentences.
// returns false if the sentence has finished playing
EXPORT bool PlayCustomSentence(CBaseToggle* ent);

// plays any active custom sentences. Call every frame for accurate word timing.
// custom sentences play outside of entity logic to avoid complicating think functions.
EXPORT void PlayCustomSentences();
