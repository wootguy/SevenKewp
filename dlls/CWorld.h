#pragma once
#include "CBaseEntity.h"

extern edict_t* g_pBodyQueueHead;

// this moved here from world.cpp, to allow classes to be derived from it
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================
class EXPORT CWorld : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );
	void loadReplacementFiles();
	void loadSentenceFiles();
	const char* getDisplayName() { return "World"; }

	string_t m_globalModelList;
	string_t m_globalSoundList;
	string_t m_sentenceFile;
	string_t m_wadlist;
	bool m_freeRoam;
};
