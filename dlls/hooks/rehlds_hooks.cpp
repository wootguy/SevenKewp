#include "rehlds.h"
#include "util.h"
#include "PluginManager.h"

#undef MAX_PACKET_ENTITIES
#include "CBasePlayer.h"

void rehlds_SendBigMessage_internal(int msgType, void* data, int sz, int playerindex) {
	IGameClient* dstClient = g_RehldsSvs->GetClient(playerindex - 1);
	sizebuf_t* dstDatagram = dstClient->GetDatagram();

	if (dstDatagram->cursize + sz + 3 >= dstDatagram->maxsize) {
		ALERT(at_console, "Message %s too large: %d\n", msgTypeStr(msgType), sz);
		return;
	}

	g_RehldsFuncs->MSG_WriteByte(dstDatagram, msgType);
	g_RehldsFuncs->MSG_WriteBuf(dstDatagram, sz, data);
}

// show a mute icon and return true if the listener muted the sender
bool handleMutedPlayer(int senderIdx, int listenerIdx, bool forceMute) {
	static float lastIconTime[33][33];

	if (Voice_GetClientListening(listenerIdx, senderIdx) && !forceMute) {
		return false;
	}

	// indicate that the player is muted
	if (gpGlobals->time - lastIconTime[senderIdx][listenerIdx] > 0.15f || lastIconTime[senderIdx][listenerIdx] > gpGlobals->time) {
		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, INDEXENT(listenerIdx));
		WRITE_BYTE(TE_PLAYERATTACHMENT);
		WRITE_BYTE(senderIdx);
		WRITE_COORD(45);
		WRITE_SHORT(MODEL_INDEX(GET_MODEL("sprites/voiceicon_m.spr")));
		WRITE_SHORT(2);
		MESSAGE_END();

		lastIconTime[senderIdx][listenerIdx] = gpGlobals->time;
	}

	return true;
}

void rehlds_SendBigMessage(int msgMode, int msgType, void* data, int sz, int playerindex) {
	if (!g_RehldsFuncs) {
		ALERT(at_console, "Rehlds API not initialized!\n");
		return;
	}

	CALL_HOOKS_VOID(pfnSendBigMessage, msgMode, msgType, data, sz, playerindex);

	bool pluginWantsMute = false;

	if (msgMode != MSG_BROADCAST) {
		if (msgType == SVC_VOICEDATA) {
			int senderIdx = ((uint8_t*)data)[0];
			CALL_HOOKS_VOID(pfnSendVoiceData, senderIdx+1, playerindex, ((uint8_t*)data) + 3, sz-3, pluginWantsMute);
			if (handleMutedPlayer(senderIdx + 1, playerindex, pluginWantsMute)) {
				return;
			}
		}

		if (playerindex > 0 && playerindex <= gpGlobals->maxClients)
			rehlds_SendBigMessage_internal(msgType, data, sz, playerindex);
		return;
	}
	
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);
		if (!plr) {
			continue;
		}

		if (msgType == SVC_VOICEDATA) {
			int senderIdx = ((uint8_t*)data)[0];
			CALL_HOOKS_VOID(pfnSendVoiceData, senderIdx + 1, playerindex, ((uint8_t*)data) + 3, sz-3, pluginWantsMute);
			if (pluginWantsMute) {
				return;
			}

			if (handleMutedPlayer(senderIdx + 1, playerindex, pluginWantsMute)) {
				continue;
			}
		}

		rehlds_SendBigMessage_internal(msgType, data, sz, i);
	}
}

void SV_ParseVoiceData_hlcoop(IGameClient* cl) {
	uint8_t chReceived[4096];
	unsigned int nDataLength = g_RehldsFuncs->MSG_ReadShort();

	if (nDataLength > sizeof(chReceived)) {
		g_RehldsFuncs->DropClient(cl, FALSE, "Invalid voice data\n");
		return;
	}

	g_RehldsFuncs->MSG_ReadBuf(nDataLength, chReceived);

	if (sv_voiceenable->value == 0.0f)
		return;

	int sender = cl->GetId();
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr) {
			continue;
		}
		
		bool pluginWantsMute = false;

		CALL_HOOKS_VOID(pfnSendVoiceData, sender + 1, i, chReceived, nDataLength, pluginWantsMute);

		if (handleMutedPlayer(sender + 1, i, pluginWantsMute)) {
			continue;
		}

		IGameClient* dstClient = g_RehldsSvs->GetClient(i - 1);
		sizebuf_t* dstDatagram = dstClient->GetDatagram();

		int nSendLength = nDataLength;
		if (sender + 1 == i && !dstClient->GetLoopback()) // voice_loopback
			nSendLength = 0;
		
		if (dstDatagram->cursize + nSendLength + 6 < dstDatagram->maxsize) {
			g_RehldsFuncs->MSG_WriteByte(dstDatagram, SVC_VOICEDATA);
			g_RehldsFuncs->MSG_WriteByte(dstDatagram, sender);
			g_RehldsFuncs->MSG_WriteShort(dstDatagram, nSendLength);
			g_RehldsFuncs->MSG_WriteBuf(dstDatagram, nSendLength, chReceived);
		}
	}
}

void Rehlds_HandleNetCommand(IRehldsHook_HandleNetCommand* chain, IGameClient* cl, uint8_t opcode) {
	const int clc_voicedata = 8;

	if (opcode == clc_voicedata) {
		SV_ParseVoiceData_hlcoop(cl);
		return;
	}

	chain->callNext(cl, opcode);
}

void Rehlds_SV_WriteFullClientUpdate(IRehldsHook_SV_WriteFullClientUpdate* chain,
	IGameClient* client, char* info, size_t maxlen, sizebuf_t* sb, IGameClient* receiver) {

	CALL_HOOKS_VOID(pfnUserInfo, receiver->GetEdict(), client->GetEdict(), info);

	chain->callNext(client, info, maxlen, sb, receiver);
}

void RegisterRehldsHooks() {
	if (!g_RehldsHookchains) {
		return;
	}
	g_RehldsHookchains->HandleNetCommand()->registerHook(&Rehlds_HandleNetCommand, HC_PRIORITY_DEFAULT + 1);
	g_RehldsHookchains->SV_WriteFullClientUpdate()->registerHook(&Rehlds_SV_WriteFullClientUpdate);
}

void UnregisterRehldsHooks() {
	if (!g_RehldsHookchains) {
		return;
	}
	g_RehldsHookchains->HandleNetCommand()->unregisterHook(&Rehlds_HandleNetCommand);
	g_RehldsHookchains->SV_WriteFullClientUpdate()->unregisterHook(&Rehlds_SV_WriteFullClientUpdate);
}