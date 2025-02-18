#include "sentences.h"
#include "util.h"
#include "fstream"
#include "CBaseToggle.h"
#include "wav.h"
#include "eng_wrappers.h"

std::unordered_map<std::string, CustomSentence> g_customSentencesMod;
std::unordered_map<std::string, CustomSentence> g_customSentencesMap;
std::unordered_map<std::string, std::vector<std::string>> g_customSentenceGroupsMod;
std::unordered_map<std::string, std::vector<std::string>> g_customSentenceGroupsMap;
std::unordered_map<std::string, CustomSentence> g_customSentences;
std::unordered_map<std::string, std::vector<std::string>> g_customSentenceGroups;
std::vector<EHANDLE> g_activeSentencePlayers;

void LoadSentenceFile(const char* path,
	std::unordered_map<std::string, CustomSentence>& sentences, 
	std::unordered_map<std::string, std::vector<std::string>>& sentenceGroups)
{
	sentences.clear();
	sentenceGroups.clear();

	std::string fpath = getGameFilePath(path);
	std::ifstream infile(fpath);

	if (fpath.empty() || !infile.is_open()) {
		ALERT(at_console, "Failed to load sentence file: %s\n", path);
		return;
	}

	int lineNum = 0;
	std::string line;
	while (std::getline(infile, line))
	{
		lineNum++;
		std::string paths[2];

		int comments = line.find("//");
		if (comments != -1) {
			line = line.substr(0, comments);
		}

		line = trimSpaces(line);
		if (line.empty()) {
			continue;
		}

		CustomSentence sent = ParseSentence(line);
		std::string sentName = toUpperCase(sent.name);
		sentences[sentName] = sent;

		std::string groupName = sentName;
		for (int i = 0; i < 2; i++) {
			if (isdigit(groupName[groupName.size() - 1])) {
				groupName = groupName.substr(0, groupName.size() - 1);
			}
			else {
				break;
			}
		}
		
		auto groupSents = sentenceGroups.find(groupName);

		if (groupSents != sentenceGroups.end()) {
			std::vector<std::string>& group = groupSents->second;

			bool isUnique = true;
			for (int i = 0; i < (int)group.size(); i++) {
				if (group[i] == sentName) {
					isUnique = false;
					break;
				}
			}

			if (isUnique) {
				group.push_back(sentName);
			}
		}
		else {
			sentenceGroups[groupName].push_back(sentName);
		}
	}
}

void CalculateAndSetPlaybackTime(SentencePart& word) {
	const char* replacedSound = g_soundReplacements.get(toLowerCase(word.file).c_str());
	std::string sound = replacedSound ? replacedSound : word.file;
	
	WavInfo wav = getWaveFileInfo(sound.c_str());
	float percentPlayed = (word.end - word.start) / 100.0f;
	float playbackSpeed = (float)word.pitch / 100.0f;
	word.duration = ((float)wav.durationMillis * percentPlayed) / playbackSpeed;
}

CustomSentence ParseSentence(std::string sentenceLine) {
	std::vector<std::string> parts = splitString(sentenceLine, " \t");

	std::string sentName = parts[0];
	std::string folder = "vox";

	int slashIdx = parts[1].find("/");
	if (slashIdx != -1) {
		folder = parts[1].substr(0, slashIdx);
		parts[1] = parts[1].substr(slashIdx + 1);
	}

	// merge space-separated parts that are enclosed in parentheses
	for (int i = 0; i < (int)parts.size(); i++) {
		if (parts[i].find("(") != std::string::npos && parts[i].find(")") == std::string::npos) {
			while (i+1 < (int)parts.size()) {
				int k = i + 1;
				parts[i] = parts[i] + " " + parts[k];
				parts.erase(parts.begin() + k);
				if (parts[i].find(")") != std::string::npos) {
					break;
				}
			}
		}
	}

	CustomSentence sent;
	sent.name = sentName;

	uint8_t globalTempo = 0;
	uint8_t globalPitch = 100;
	uint8_t globalStart = 0;
	uint8_t globalEnd = 100;
	uint8_t globalVol = 100;

	for (int i = 1; i < (int)parts.size(); i++) {
		SentencePart part;
		part.tempo = globalTempo;
		part.pitch = globalPitch;
		part.start = globalStart;
		part.end = globalEnd;
		part.volume = globalVol;
		part.duration = 0;

		std::string originalPart = parts[i];

		bool hasPeriod = parts[i].find(".") != std::string::npos;
		bool hasComma = parts[i].find(",") != std::string::npos;

		if (hasPeriod) {
			parts[i] = replaceString(parts[i], ".", "");
		}
		if (hasComma) {
			parts[i] = replaceString(parts[i], ",", "");
		}

		int openParenIdx = parts[i].find("(");
		int closeParenIdx = parts[i].find(")");
		bool isMalformed = false;

		std::string word = parts[i];

		if (openParenIdx != -1 && closeParenIdx != -1) {
			word = parts[i].substr(0, openParenIdx);
			std::string modifierStr = parts[i].substr(openParenIdx+1, closeParenIdx - openParenIdx);

			std::vector<std::string> modifiers = splitString(modifierStr, " \t");

			for (int k = 0; k < (int)modifiers.size(); k++) {
				std::string modifier = toLowerCase(modifiers[k]);

				if (modifier.size() < 2) {
					isMalformed = true;
					continue;
				}

				char mtype = modifier[0];
				int modVal = atoi(modifier.substr(1).c_str());

				if (mtype == 'e') {
					part.end = modVal;
				}
				else if (mtype == 'p') {
					part.pitch = modVal;
				}
				else if (mtype == 's') {
					part.start = modVal;
				}
				else if (mtype == 'v') {
					part.volume = modVal;
				}
				else if (mtype == 't') {
					part.tempo = modVal;
				}
				else {
					isMalformed = true;
				}
			}
		}
		else if (openParenIdx != -1 || closeParenIdx != -1) {
			isMalformed = true;
		}

		if (word.empty()) {
			// sentence part didn't have any text, only modifiers.
			// so, future words will use the settings that were specified
			globalTempo = part.tempo;
			globalPitch = part.pitch;
			globalStart = part.start;
			globalEnd = part.end;
			globalVol = part.volume;
		}
		else {
			part.file = folder + "/" + word + ".wav";
			sent.words.push_back(part);
		}

		if (hasPeriod) {
			part.file = folder + "/_period.wav";
			sent.words.push_back(part);
		}

		if (hasComma) {
			part.file = folder + "/_comma.wav";
			sent.words.push_back(part);
		}

		if (isMalformed) {
			ALERT(at_console, "Malformed sentence part in '%s' at '%s'\n", sentName.c_str(), originalPart.c_str());
		}
	}

	return sent;
}

CustomSentence* GetCustomSentence(std::string sentenceName) {
	auto sent = g_customSentences.find(toUpperCase(sentenceName));
	return sent == g_customSentences.end() ? NULL : &sent->second;
}

CustomSentence* GetRandomCustomSentence(std::string sentenceGroupName) {
	auto sentGroup = g_customSentenceGroups.find(toUpperCase(sentenceGroupName));

	if (sentGroup == g_customSentenceGroups.end()) {
		return NULL;
	}

	std::vector<std::string>& groupVec = sentGroup->second;

	if (sentGroup->second.empty()) {
		ALERT(at_console, "Custom sentence group is empty '%s'\n", sentenceGroupName.c_str());
		return NULL;
	}

	std::string randomSentName = groupVec[RANDOM_LONG(0, groupVec.size()-1)];

	CustomSentence* sent = GetCustomSentence(randomSentName);

	if (!sent) {
		ALERT(at_console, "Invalid custom sentence selected randomly '%s'\n", randomSentName.c_str());
	}

	return sent;
}

void AddCustomSentencePlayer(CBaseToggle* ent, CustomSentence* sent, float volume, float attn) {
	if (ent) {
		ent->m_customSent = sent;
		ent->m_customSentLastWord = -1;
		ent->m_customSentStartTime = g_engfuncs.pfnTime();
		ent->m_customSentAttn = attn;
		ent->m_customSentVol = volume;

		bool isUnique = true;
		for (int i = 0; i < (int)g_activeSentencePlayers.size(); i++) {
			if (g_activeSentencePlayers[i].GetEntity() == ent) {
				isUnique = false;
				break;
			}
		}

		if (isUnique)
			g_activeSentencePlayers.push_back(EHANDLE(ent->edict()));
	}
}

// precache a specific sentence
bool PrecacheCustomSentence_internal(CBaseEntity* ent, CustomSentence* sent) {
	if (!sent) {
		return false;
	}

	for (SentencePart& part : sent->words) {
		// TODO: per-monster sound replacement won't work here, because the duration of the sound clips
		// is stored globally in the sentence data. So, passing NULL here for now
		PRECACHE_SOUND_ENT(NULL, part.file.c_str());
		CalculateAndSetPlaybackTime(part);
	}

	return true;
}

bool PrecacheCustomSentence(CBaseEntity* ent, std::string name) {
	name = toUpperCase(name);

	if (name[0] != '!') {
		auto sentGroup = g_customSentenceGroups.find(toUpperCase(name));

		if (sentGroup == g_customSentenceGroups.end()) {
			return false;
		}
		
		bool allSuccess = true;

		for (const std::string& item : sentGroup->second) {
			CustomSentence* sent = GetCustomSentence(item);
			if (sent) {
				PrecacheCustomSentence_internal(ent, sent);
			}
			else {
				allSuccess = false;
				ALERT(at_console, "PrecacheCustomSentence failed: Group has invalid member %s\n", item.c_str());
			}
		}

		return allSuccess;
	}
	else {
		CustomSentence* sent = GetCustomSentence(name.substr(1));
		if (sent) {
			return PrecacheCustomSentence_internal(ent, sent);
		}
	}

	return false;
}

bool PlayCustomSentence(CBaseToggle* ent) {
	if (!ent) {
		return false;
	}

	CustomSentence* sent = ent->m_customSent;

	if (!sent || ent->m_customSentLastWord == (int)sent->words.size() - 1) {
		ent->m_customSent = NULL;
		
		// this technically isn't the end of sound playback, but nothing needs to know that yet
		return false;
	}

	float t = g_engfuncs.pfnTime() - ent->m_customSentStartTime;
	float sentTime = 0;

	SentencePart* playWord = NULL;

	for (int i = 0; i < (int)sent->words.size(); i++) {
		SentencePart& part = sent->words[i];

		if (t >= sentTime && i > ent->m_customSentLastWord) {
			ent->m_customSentLastWord = i;
			playWord = &part;
		}

		sentTime += (float)part.duration / 1000.0f;
	}

	if (playWord) {
		const char* sound = playWord->file.c_str();
		float vol = (playWord->volume / 100.0f) * ent->m_customSentVol;
		float attn = ent->m_customSentAttn;
		EMIT_SOUND_DYN(ent->edict(), CHAN_VOICE, sound, vol, attn, 0, playWord->pitch);
	}

	return true; // still playing
}

void PlayCustomSentences() {
	for (int i = 0; i < (int)g_activeSentencePlayers.size(); i++) {
		CBaseEntity* ent = g_activeSentencePlayers[i].GetEntity();
		CBaseToggle* tog = ent ? ent->MyTogglePointer() : NULL;

		if (!tog || !PlayCustomSentence(tog)) {
			g_activeSentencePlayers.erase(g_activeSentencePlayers.begin() + i);
			i--;
			continue;
		}
	}
}