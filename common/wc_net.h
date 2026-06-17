#pragma once

class CWeaponCustom;

enum PredictionDataSendMode {
	WC_PRED_SEND_INIT,	// initialize prediction data for first pickup
	WC_PRED_SEND_WEP,	// only send weapon prediction data
	WC_PRED_SEND_EVT,	// only send event prediction data
	WC_PRED_SEND_BOTH	// send weapon and event data
};

extern uint32_t g_wcPredDataSent[MAX_WEAPONS]; // bitfields indicating which players received prediction data

EXPORT void UTIL_SendCustomWeaponPredictionData(edict_t* target, CWeaponCustom* wep, PredictionDataSendMode sendMode);

EXPORT bool UTIL_HasCustomWeaponPredictionData(edict_t* target, CWeaponCustom* wep);

// client utils
int UTIL_ReadCustomWeaponPredictionData(const char* pszName, int iSize, void* pbuf);
int UTIL_ReadCustomWeaponPredictionAttackData(const char* pszName, int iSize, void* pbuf);
int UTIL_ReadCustomWeaponPredictionEventData(const char* pszName, int iSize, void* pbuf);