#include "sentences.h"
#include "util.h"
#include "fstream"
#include "CBaseToggle.h"
#include "wav.h"
#include "eng_wrappers.h"

CustomSentences g_customSentencesMod;
CustomSentences g_customSentencesMap;
CustomSentences g_customSentences;
std::vector<EHANDLE> g_activeSentencePlayers;

void LoadSentenceFile(const char* path, CustomSentences& sentences)
{
	sentences.clear();

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

		CustomSentence sent = ParseSentence(sentences.strings, line);
		std::string sentName = toUpperCase(sent.name.str());
		sentences.map.put(sentName.c_str(), sent);

		std::string groupName = sentName;
		for (int i = 0; i < 2; i++) {
			if (isdigit(groupName[groupName.size() - 1])) {
				groupName = groupName.substr(0, groupName.size() - 1);
			}
			else {
				break;
			}
		}
		
		SentenceGroup* group = sentences.groups.get(groupName.c_str());

		if (group) {

			bool isUnique = true;
			for (int i = 0; i < (int)group->numSents; i++) {
				if (group->sents[i].str() == sentName) {
					isUnique = false;
					break;
				}
			}

			if (isUnique) {
				if (group->numSents < MAX_GROUP_SENTS) {
					group->sents[group->numSents++] = sentences.strings.alloc(sentName.c_str());
				}
				else {
					ALERT(at_error, "Exceeded max sentence group members for: %s\n", groupName.c_str());
				}
			}
		}
		else {
			SentenceGroup group;
			memset(&group, 0, sizeof(SentenceGroup));
			group.sents[group.numSents++] = sentences.strings.alloc(sentName.c_str());

			sentences.groups.put(groupName.c_str(), group);
		}
	}
}

void CalculateAndSetPlaybackTime(SentencePart& word) {
	const char* replacedSound = g_soundReplacements.get(toLowerCase(word.file.str()).c_str());
	const char* sound = replacedSound ? replacedSound : word.file.str();
	
	WavInfo wav = getWaveFileInfo(sound);
	float percentPlayed = (word.end - word.start) / 100.0f;
	float playbackSpeed = (float)word.pitch / 100.0f;
	word.duration = ((float)wav.durationMillis * percentPlayed) / playbackSpeed;
}

CustomSentence ParseSentence(StringPool& stringPool, std::string sentenceLine) {
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
	memset(&sent, 0, sizeof(CustomSentence));
	sent.name = stringPool.alloc(sentName.c_str());

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
			part.file = stringPool.alloc((folder + "/" + word + ".wav").c_str());
			sent.words[sent.numWords++] = part;
		}

		if (isMalformed) {
			ALERT(at_console, "Malformed sentence part in '%s' at '%s'\n", sentName.c_str(), originalPart.c_str());
		}

		if (sent.numWords < MAX_SENTENCE_WORDS) {
			if (hasPeriod) {
				part.file = stringPool.alloc((folder + "/_period.wav").c_str());
				sent.words[sent.numWords++] = part;
			}

			if (hasComma) {
				part.file = stringPool.alloc((folder + "/_comma.wav").c_str());
				sent.words[sent.numWords++] = part;
			}
		}

		if (sent.numWords >= MAX_SENTENCE_WORDS) {
			ALERT(at_error, "Max sentence words exceeded for %s\n", sentName.c_str());
			break;
		}
	}

	return sent;
}

CustomSentence* GetCustomSentence(std::string sentenceName) {
	return g_customSentences.map.get(toUpperCase(sentenceName).c_str());
}

CustomSentence* GetRandomCustomSentence(std::string sentenceGroupName) {
	SentenceGroup* sentGroup = g_customSentences.groups.get(toUpperCase(sentenceGroupName).c_str());

	if (!sentGroup) {
		return NULL;
	}

	if (!sentGroup->numSents) {
		ALERT(at_console, "Custom sentence group is empty '%s'\n", sentenceGroupName.c_str());
		return NULL;
	}

	std::string randomSentName = sentGroup->sents[RANDOM_LONG(0, sentGroup->numSents-1)].str();

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

	for (int i = 0; i < sent->numWords; i++) {
		// TODO: per-monster sound replacement won't work here, because the duration of the sound clips
		// is stored globally in the sentence data. So, passing NULL here for now
		SentencePart& part = sent->words[i];
		PRECACHE_SOUND_ENT(NULL, part.file.str());
		CalculateAndSetPlaybackTime(part);
	}

	return true;
}

bool PrecacheCustomSentence(CBaseEntity* ent, std::string name) {
	name = toUpperCase(name);

	if (name[0] != '!') {
		SentenceGroup* sentGroup = g_customSentences.groups.get(toUpperCase(name).c_str());

		if (!sentGroup) {
			return false;
		}
		
		bool allSuccess = true;
		for (int i = 0; i < sentGroup->numSents; i++) {
			CustomSentence* sent = GetCustomSentence(sentGroup->sents[i].str());
			if (sent) {
				PrecacheCustomSentence_internal(ent, sent);
			}
			else {
				allSuccess = false;
				ALERT(at_console, "PrecacheCustomSentence failed: Group has invalid member %s\n",
					sentGroup->sents[i].str());
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

	if (!sent || ent->m_customSentLastWord == (int)sent->numWords - 1) {
		ent->m_customSent = NULL;
		
		// this technically isn't the end of sound playback, but nothing needs to know that yet
		return false;
	}

	float t = g_engfuncs.pfnTime() - ent->m_customSentStartTime;
	float sentTime = 0;

	SentencePart* playWord = NULL;

	for (int i = 0; i < (int)sent->numWords; i++) {
		SentencePart& part = sent->words[i];

		if (t >= sentTime && i > ent->m_customSentLastWord) {
			ent->m_customSentLastWord = i;
			playWord = &part;
		}

		sentTime += (float)part.duration / 1000.0f;
	}

	if (playWord) {
		const char* sound = playWord->file.str();
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