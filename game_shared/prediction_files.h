#include <stdint.h>

extern const char* g_prediction_files[];
extern const int g_prediction_files_sz;
extern int g_soundvariety; // used client-side to select random sounds

extern uint8_t g_predMsgData[190];
extern int g_predMsgLen;

// generate replacement indexes for sounds/models predicted by the client (call after everything is precached)
void GeneratePredicionData();

// called by the client to get the replacement file used by the server
const char* RemapFile(const char* path);

// called by the client
void HookPredictionMessages();