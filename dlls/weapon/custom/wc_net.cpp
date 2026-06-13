#include "extdll.h"
#include "util.h"
#include "user_messages.h"
#include "CWeaponCustom.h"

#ifdef CLIENT_DLL
#include "../cl_dll/hud.h"
#include "parsemsg.h"
void InitCustomWeapon(int id);
CustomWeaponParams* GetCustomWeaponParams(int id);
#define PRINTF(msg, ...) gEngfuncs.Con_Printf(msg, ##__VA_ARGS__)
#define PRINTD(msg, ...) gEngfuncs.Con_DPrintf(msg, ##__VA_ARGS__)
#endif

#define MAX_NET_MESSAGE_SIZE 192

using namespace std;

uint32_t g_wcPredDataSent[MAX_WEAPONS];

int g_evt_data_chunk_size = 8; // no more than 16 to fit in a header byte


//
// Functions for reading/writing prediction network messages
//

uint32_t wc_get_bits(void* dat, int bits) {
	uint32_t mask = ((1 << bits) - 1);

	if (bits > 16) {
		return *((uint32_t*)dat) & mask;
	}
	else if (bits > 8) {
		return *((uint16_t*)dat) & mask;
	}
	return *((uint8_t*)dat) & mask;
}

int wc_send_netmsg_struct(struct_desc_t& desc, void* dat) {
	int byteCount = 0;

#ifndef CLIENT_DLL
	uint32_t packedFields = 0;
	int packedBitCount = 0;
	ALERT(at_aiconsole, "Send net struct '%s' (%d fields)\n", desc.name, desc.numFields);

	for (int i = 0; i < desc.numFields; i++) {
		field_desc_t& field = desc.fields[i];
		uint8_t* fieldDat = ((uint8_t*)dat) + field.offset;

		if (field.flags & FL_FIELD_NO_NETWORK) {
			ALERT(at_aiconsole, "    %-20s = (skipped server-side field)\n", field.name);
			continue;
		}

		if (field.conditionByteOfs) {
			uint8_t* condDat = ((uint8_t*)dat) + field.conditionByteOfs;
			if (*condDat == 0) {
				ALERT(at_aiconsole, "    %-20s = (skipped by condition byte %d)\n", field.name, (int)field.conditionByteOfs);
				continue;
			}
		}

		int bytes = wc_get_field_bytes(field);
		bool isBitPacked = field.bits != 0 && field.bits != bytes * 8;

		if (g_developer->value) {
			string fieldStr = wc_get_field_str(field, fieldDat);
			if (isBitPacked) {
				ALERT(at_aiconsole, "    %-20s = %-24s (%d bits)\n", field.name, fieldStr.c_str(), field.bits);
			}
			else {
				ALERT(at_aiconsole, "    %-20s = %-24s (%d bytes)\n", field.name, fieldStr.c_str(), bytes);
			}
		}

		if (isBitPacked) {
			uint32_t appendVal = wc_get_bits(fieldDat, field.bits);
			packedFields |= appendVal << packedBitCount;
			packedBitCount += field.bits;

			if (packedBitCount % 8 == 0) {
				bytes = packedBitCount / 8;
				WRITE_BYTES((uint8_t*)&packedFields, bytes);
				byteCount += bytes;
				packedFields = 0;
				packedBitCount = 0;
			}
			continue;
		}
		
		if (packedBitCount != 0) {
			ALERT(at_error, "bit packed fields not byte aligned for %s at %s\n", desc.name, field.name);
			return byteCount;
		}
		switch (field.type) {
		case WC_PARAM_STRING:
			WRITE_STRING(STRING(*(string_t*)dat));
			byteCount += 1 + strlen(STRING(*(string_t*)dat));
			break;
		case WC_PARAM_UINT8_ARRAY_8: {
			WepEvtArr8* arr = (WepEvtArr8*)fieldDat;
			WRITE_BYTE(arr->arrSz);
			WRITE_BYTES(arr->arr, arr->arrSz);
			byteCount += 1 + arr->arrSz;
			break;
		}
		case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2: {
			WepEvtArr16* arr = (WepEvtArr16*)fieldDat;
			WRITE_BYTE(arr->arrSz);
			WRITE_BYTES((uint8_t*)arr->arr, arr->arrSz * sizeof(uint16_t));
			byteCount += 1 + arr->arrSz * sizeof(uint16_t);
			break;
		}
		default:
			WRITE_BYTES(fieldDat, bytes);
			byteCount += bytes;
			break;
		}
	}

	if (packedBitCount) {
		ALERT(at_error, "bit packed fields not byte aligned for %s\n", desc.name);
	}

	ALERT(at_aiconsole, "Wrote %d bytes\n\n", byteCount);

#endif
	return byteCount;
}

void READ_BYTES(uint8_t* bytes, int count) {
#ifdef CLIENT_DLL
	int longCount = count / 4;
	int byteCount = count % 4;

	int32_t* longPtr = (int32_t*)bytes;
	for (int i = 0; i < longCount; i++) {
		*longPtr = READ_LONG();
		longPtr++;
	}

	uint8_t* bytePtr = bytes + longCount * 4;
	for (int i = 0; i < byteCount; i++) {
		*bytePtr = READ_BYTE();
		bytePtr++;
	}
#endif
}

void wc_read_netmsg_struct(struct_desc_t& desc, void* dat, bool isEvent=false) {
#ifdef CLIENT_DLL
	int packedBitCount = 0;
	int bitPackFieldStartIdx = -1;
	int byteCount = 0;
	bool verbose = gHUD.m_Debug.m_HUD_debug->value;
	if (verbose) {
		if (isEvent) {
			PRINTD("\nParse event %s (%d fields)\n", describe_event(*(WepEvt*)dat), desc.numFields);
		}
		else {
			PRINTD("\nParse net struct '%s' (%d fields)\n", desc.name, desc.numFields);
		}
	}

	for (int i = 0; i < desc.numFields; i++) {
		field_desc_t& field = desc.fields[i];
		uint8_t* fieldDat = ((uint8_t*)dat) + field.offset;

		if (field.flags & FL_FIELD_NO_NETWORK) {
			if (verbose) PRINTD("    %-20s = (skipped server-side field)\n", field.name);
			continue;
		}

		if (field.conditionByteOfs) {
			uint8_t* condDat = ((uint8_t*)dat) + field.conditionByteOfs;
			if (*condDat == 0) {
				if (verbose) PRINTD("    %-20s = (skipped by condition byte %d)\n", field.name, (int)field.conditionByteOfs);
				continue;
			}
		}

		int bytes = wc_get_field_bytes(field);
		bool isBitPacked = field.bits != 0 && field.bits != bytes * 8;

		// read bit-packed fields into a buffer until a regular field is read
		if (isBitPacked) {
			if (bitPackFieldStartIdx < 0)
				bitPackFieldStartIdx = i;

			packedBitCount += field.bits;

			if (packedBitCount % 8 != 0) {
				continue; // keep reading packed fields until byte alignment
			}
		}
		
		if (packedBitCount) {
			if (packedBitCount % 8 == 0) {
				uint32_t packedFields = 0;
				if (packedBitCount == 32) {
					packedFields = (uint32_t)READ_LONG();
					byteCount += 4;
				}
				else if (packedBitCount == 16) {
					packedFields = (uint32_t)READ_SHORT();
					byteCount += 2;
				}
				else if (packedBitCount == 8) {
					packedFields = (uint32_t)READ_BYTE();
					byteCount += 1;
				}
				else {
					PRINTF("Invalid bit byte alignment %d\n", packedBitCount);
					return;
				}

				for (int k = bitPackFieldStartIdx; k <= i; k++) {
					field_desc_t& packedField = desc.fields[k];
					uint8_t* packedFieldDat = ((uint8_t*)dat) + packedField.offset;

					// read bit packed data out of the buffer
					uint64_t val = packedFields & ((1ULL << packedField.bits) - 1ULL);

					if (field.bits > 16)
						*(uint32_t*)packedFieldDat = (uint32_t)val;
					else if (field.bits > 8)
						*(uint16_t*)packedFieldDat = (uint16_t)val;
					else
						*(uint8_t*)packedFieldDat = (uint8_t)val;

					if (verbose) {
						string fieldStr = wc_get_field_str(packedField, packedFieldDat);
						PRINTD("    %-20s = %-24s (%d bits)\n", packedField.name, fieldStr.c_str(), packedField.bits);
					}

					packedFields >>= packedField.bits;
					packedBitCount -= packedField.bits;
				}

				packedFields = 0;
				packedBitCount = 0;
				bitPackFieldStartIdx = -1;
				continue;
			}
			else {
				PRINTF("bit packed fields not byte aligned for %s\n", desc.name);
				return;
			}
		}

		switch (field.type) {
		case WC_PARAM_STRING: {
			const char* temp = READ_STRING();
			byteCount += strlen(temp) + 1;
			PRINTF("String parsing not implemented\n");
			break;
		}
		case WC_PARAM_UINT8_ARRAY_8: {
			uint8_t sz = READ_BYTE();
			*fieldDat = sz;
			READ_BYTES(fieldDat + 1, sz);
			byteCount += 1 + sz;
			break;
		}
		case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2: {
			uint8_t sz = READ_BYTE();
			*fieldDat = sz;
			READ_BYTES(fieldDat + 1, sz*sizeof(uint16_t));
			byteCount += 1 + sz * sizeof(uint16_t);
			break;
		}
		default:
			READ_BYTES(fieldDat, bytes);
			byteCount += bytes;
			break;
		}

		if (verbose) {
			string fieldStr = wc_get_field_str(field, fieldDat);
			PRINTD("    %-20s = %-24s (%d bytes)\n", field.name, fieldStr.c_str(), bytes);
		}
	}

	if (verbose)
		PRINTD("Read %d bytes\n", byteCount);
#endif
}

void SendWeaponData(edict_t* target, CWeaponCustom* wep) {
#ifndef CLIENT_DLL
	CustomWeaponParams& params = wep->params;
	uint8_t* dat = (uint8_t*)&params;

	MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeapon, NULL, target);
	WRITE_BYTE(wep->m_iId);

	wc_send_netmsg_struct(g_wc_desc_general, dat);

	for (int k = 0; k < 2; k++) {
		wc_send_netmsg_struct(g_wc_desc_ammo, dat + sizeof(WeaponCustomAmmoInfo) * k);
	}

	for (int k = 0; k < 3; k++) {
		wc_send_netmsg_struct(g_wc_desc_reload, dat + sizeof(WeaponCustomReload) * k);

		if (k == 2 && !(params.flags & FL_WC_WEP_SHOTGUN_RELOAD))
			break;
	}

	if (params.flags & FL_WC_WEP_AKIMBO) {
		wc_send_netmsg_struct(g_wc_desc_akimbo_reload, dat);
		wc_send_netmsg_struct(g_wc_desc_akimbo, dat);
	}

	if (params.flags & FL_WC_WEP_HAS_LASER) {
		wc_send_netmsg_struct(g_wc_desc_laser, dat);
	}

	if (params.flags & FL_WC_WEP_HAS_STATE_SPRITE) {
		wc_send_netmsg_struct(g_wc_desc_state_sprite, dat);
	}

	for (int k = 0; k < 4; k++) {
		if (!(params.flags & FL_WC_WEP_HAS_PRIMARY) && k == 0)
			continue;
		if (!(params.flags & FL_WC_WEP_HAS_SECONDARY) && k == 1)
			continue;
		if (!(params.flags & FL_WC_WEP_HAS_TERTIARY) && k == 2)
			continue;
		if (!(params.flags & FL_WC_WEP_HAS_ALT_PRIMARY) && k == 3)
			continue;

		wc_send_netmsg_struct(g_wc_desc_shoot_opts, dat + sizeof(CustomWeaponShootOpts) * k);
	}
	MESSAGE_END();
#endif
}

int SendEventData(edict_t* target, CWeaponCustom* wep) {
	int evBytes = 0;

#ifndef CLIENT_DLL
	CustomWeaponParams& params = wep->params;

	int chunks = ceilf(params.numEvents / (float)g_evt_data_chunk_size);

	for (int c = 0; c < chunks; c++) {
		int evtOffset = c * g_evt_data_chunk_size;
		if (evtOffset >= params.numEvents)
			break;

		MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeaponEvents, NULL, target);
		WRITE_BYTE(wep->m_iId);

		int sendEvents = 0; // TODO: skipping events will complicate live updates due to mismatched indexes
		for (int k = evtOffset; k < params.numEvents && k - evtOffset < g_evt_data_chunk_size; k++) {
			WepEvt& evt = params.events[k];

			struct_desc_t* desc = get_evt_desc(evt.evtType);
			if (desc && !is_server_side_event(evt.evtType))
				sendEvents++;
		}

		WRITE_BYTE((c << 4) | (sendEvents - 1));

		for (int k = evtOffset; k < params.numEvents && k - evtOffset < g_evt_data_chunk_size; k++) {
			WepEvt& evt = params.events[k];

			if (is_server_side_event(evt.evtType))
				continue;

			struct_desc_t* desc = get_evt_desc(evt.evtType);
			if (!desc)
				continue;

			int headerSz = 2;
			uint16_t condFlags = (evt.hasOffset << 2) | (evt.hasDelay << 1) | evt.hasTrigArg;
			uint16_t packedHeader = (condFlags << (EVT_TYPE_BITS + EVT_TRIGGER_BITS)) | (evt.trigger << EVT_TYPE_BITS) | evt.evtType;

			WRITE_SHORT(packedHeader);
			if (evt.hasTrigArg) {
				WRITE_BYTE(evt.triggerArg);
				headerSz += 1;
			}
			if (evt.hasDelay) {
				WRITE_SHORT(evt.delay);
				headerSz += 2;
			}
			if (evt.hasOffset) {
				WRITE_SHORT(evt.offset);
				headerSz += 2;
			}

			ALERT(at_aiconsole, "Write event %s (%d header bytes)\n",
				describe_event(evt), headerSz);

			wc_send_netmsg_struct(*desc, &evt);
		}
		MESSAGE_END();

		evBytes += LastMsgSize();
	}

#endif

	return evBytes;
}

void UTIL_SendCustomWeaponPredictionData(edict_t* target, CWeaponCustom* wep, PredictionDataSendMode sendMode) {
#ifndef CLIENT_DLL
	CustomWeaponParams& params = wep->params;

	if (params.flags & FL_WC_WEP_NO_PREDICTION) {
		return;
	}

	if (UTIL_HasCustomWeaponPredictionData(target, wep) && sendMode == WC_PRED_SEND_INIT) {
		//ALERT(at_console, "PLayer already has the prediction data\n");
		return;
	}

	if (sendMode != WC_PRED_SEND_EVT) {
		SendWeaponData(target, wep);
	}

	int mainBytes = LastMsgSize();

	int evBytes = 0;

	if (sendMode != WC_PRED_SEND_WEP) {
		evBytes += SendEventData(target, wep);
	}

	ALERT(at_console, "Sent %d prediction bytes for %s (%d + %d evt)\n",
		evBytes + mainBytes, STRING(wep->pev->classname), mainBytes, evBytes);

	g_wcPredDataSent[wep->m_iId] |= PLRBIT(target);
#endif
}

bool UTIL_HasCustomWeaponPredictionData(edict_t* target, CWeaponCustom* wep) {
	return g_wcPredDataSent[wep->m_iId] & PLRBIT(target);
}

int UTIL_ReadCustomWeaponPredictionData(const char* pszName, int iSize, void* pbuf) {
#ifdef CLIENT_DLL
	BEGIN_READ(pbuf, iSize);

	int weaponId = READ_BYTE();

	if (weaponId < 0 || weaponId >= MAX_WEAPONS)
		return 0;


	InitCustomWeapon(weaponId);
	CustomWeaponParams& parms = *GetCustomWeaponParams(weaponId);
	memset(&parms, 0, sizeof(CustomWeaponParams));
	uint8_t* dat = (uint8_t*)&parms;

	wc_read_netmsg_struct(g_wc_desc_general, dat);

	for (int k = 0; k < 2; k++) {
		wc_read_netmsg_struct(g_wc_desc_ammo, dat + sizeof(WeaponCustomAmmoInfo) * k);
	}

	for (int k = 0; k < 3; k++) {
		wc_read_netmsg_struct(g_wc_desc_reload, dat + sizeof(WeaponCustomReload)*k);

		if (k == 2 && !(parms.flags & FL_WC_WEP_SHOTGUN_RELOAD))
			break;
	}

	if (parms.flags & FL_WC_WEP_AKIMBO) {
		wc_read_netmsg_struct(g_wc_desc_akimbo_reload, dat);
		wc_read_netmsg_struct(g_wc_desc_akimbo, dat);
	}

	if (parms.flags & FL_WC_WEP_HAS_LASER) {
		wc_read_netmsg_struct(g_wc_desc_laser, dat);
	}

	if (parms.flags & FL_WC_WEP_HAS_STATE_SPRITE) {
		wc_read_netmsg_struct(g_wc_desc_state_sprite, dat);
	}

	for (int i = 0; i < 4; i++) {
		if (!(parms.flags & FL_WC_WEP_HAS_PRIMARY) && i == 0)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_SECONDARY) && i == 1)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_TERTIARY) && i == 2)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_ALT_PRIMARY) && i == 3)
			continue;

		wc_read_netmsg_struct(g_wc_desc_shoot_opts, dat + sizeof(CustomWeaponShootOpts) * i);
	}

#endif
	return 1;
}

int UTIL_ReadCustomWeaponPredictionEventData(const char* pszName, int iSize, void* pbuf) {
#ifdef CLIENT_DLL
	BEGIN_READ(pbuf, iSize);

	int weaponId = READ_BYTE();

	if (weaponId < 0 || weaponId >= MAX_WEAPONS)
		return 0;

	CustomWeaponParams& parms = *GetCustomWeaponParams(weaponId);

	uint8_t packetHeader = READ_BYTE();
	int evtIdxOffset = (packetHeader >> 4) * g_evt_data_chunk_size;
	int packetEvents = (packetHeader & 0xf) + 1;
	parms.numEvents = V_max(parms.numEvents, evtIdxOffset + packetEvents);
	if (parms.numEvents >= MAX_WC_EVENTS) {
		return 0;
	}

	for (int i = evtIdxOffset; i < parms.numEvents && i < evtIdxOffset + g_evt_data_chunk_size; i++) {
		uint16_t packedHeader = READ_SHORT();
		WepEvt& evt = parms.events[i];
		memset(&evt, 0, sizeof(WepEvt));

		int headerSz = 2;
		evt.evtType = packedHeader & ((1 << EVT_TYPE_BITS) - 1);
		evt.trigger = (packedHeader >> EVT_TYPE_BITS) & ((1 << EVT_TRIGGER_BITS) - 1);
		evt.hasTrigArg = (packedHeader >> 12) & 1;
		evt.hasDelay = (packedHeader >> 13) & 1;
		evt.hasOffset = (packedHeader >> 14) & 1;

		if (evt.hasTrigArg) {
			evt.triggerArg = READ_BYTE();
			headerSz += 1;
		}
		if (evt.hasDelay) {
			evt.delay = READ_SHORT();
			headerSz += 2;
		}
		if (evt.hasOffset) {
			evt.offset = READ_SHORT();
			headerSz += 2;
		}

		struct_desc_t* desc = get_evt_desc(evt.evtType);
		if (!desc) {
			PRINTF("Bad custom weapon event type %d\n", (int)evt.evtType);
			continue;
		}

		wc_read_netmsg_struct(*desc, &evt, true);
	}
#endif
	return 1;
}