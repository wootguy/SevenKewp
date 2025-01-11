import os, sys, codecs

if len(sys.argv) <= 1:
	print("Usage: python convert_angelscript.py <file_or_folder>\n")
	print("This script eases the conversion from angelscript to HL Co-op plugins by doing a find+replace")
	print("for common syntax and function differences. Pass a folder name to convert all the scripts inside.")
	sys.exit()

path = sys.argv[1]

terms = [
	('@', '*'),
	('.pev.', '->pev->'),
	('.edict()', '->edict()'),
	('g_EntityFuncs.Remove', 'RemoveEntity'),
	('g_EntityFuncs.CreateEntity', 'CreateEntity'),
	('cast<CBasePlayer*>', '(CBasePlayer*)'),
	('.insertLast', '.push_back'),
	('.entindex()', '->entindex()'),
	( '!is null', ''),
	( 'is null', ' == NULL '),
	( '.IsBSPModel()', '->IsBSPModel()'),
	('g_Engine.', 'gpGlobals->'),
	('g_Utility.TraceLine', 'TRACE_LINE'),
	('Math.MakeVectors', 'MAKE_VECTORS'),
	('null;', 'NULL;'),
	('null,', 'NULL,'),
	('null)', 'NULL)'),
	('array<', 'vector<'),
	('g_SoundSystem.PlaySound', 'EMIT_SOUND_DYN'),
	('g_SoundSystem.EmitSoundDyn', 'EMIT_SOUND_DYN'),
	('Math.RandomLong', 'RANDOM_LONG'),
	('Math.RandomFloat', 'RANDOM_FLOAT'),
	('g_PlayerFuncs.SharedRandomFloat', 'RANDOM_FLOAT'),
	('g_PlayerFuncs.SharedRandomLong', 'RANDOM_LONG'),
	('g_PlayerFuncs.ClientPrint', 'UTIL_ClientPrint'),
	('g_PlayerFuncs.ClientPrintAll', 'UTIL_ClientPrintAll'),
	('g_WeaponFuncs.DecalGunshot', 'DecalGunshot'),
	('.IsAlive()', '->IsAlive()'),
	('.IsConnected()', '->IsConnected()'),
	(' and ', ' && '),
	(' or ', ' || '),
	('.GetObserver().IsObserver()', '->IsObserver()'),
	('g_PlayerFuncs.SayTextAll(plr,', 'ClientPrintAll(HUD_PRINTTALK,'),
	('@ent = g_EntityFuncs.FindEntityByTargetname(ent,', 'ent = FIND_ENTITY_BY_STRING(ent, "targetname",'),
	('@ent = g_EntityFuncs.FindEntityByClassname(ent,',  'ent = FIND_ENTITY_BY_STRING(ent, "classname",'),
	('DateTime', 'uint64_t'),
	('g_EngineFuncs.GetPlayerAuthId', 'GetPlayerUniqueId'),
	('g_Game.PrecacheModel', 'PRECACHE_MODEL'),
	('g_SoundSystem.PrecacheSound', 'PRECACHE_SOUND'),
	('self.', ''),
	('self->', ''),
	('g_ItemRegistry.RegisterWeapon', 'UTIL_RegisterWeapon'),
	('g_EntityFuncs.SetModel(self', 'SET_MODEL(edict()'),
	('g_EntityFuncs.SetModel', 'SET_MODEL'),
	('g_EntityFuncs.EjectBrass', 'EjectBrass'),
	('g_EntityFuncs.Instance', 'CBaseEntity::Instance'),
	('g_EntityFuncs.SetSize', 'UTIL_SetSize'),
	('g_EntityFuncs.SetOrigin', 'UTIL_SetOrigin'),
	('g_SoundSystem.EmitSound', 'EMIT_SOUND'),
	('g_Game.PrecacheOther', 'UTIL_PrecacheOther'),
	('Math.VecToAngles', 'UTIL_VecToAngles'),
	('g_Utility.GetCircularGaussianSpread', 'GetCircularGaussianSpread'),
	('bool GetItemInfo(ItemInfo& out info)', 'int GetItemInfo(ItemInfo* info)'),
	('bool GetItemInfo(ItemInfo& info)', 'int GetItemInfo(ItemInfo* info)'),
	('bool AddToPlayer(CBasePlayer* pPlayer)', 'int AddToPlayer(CBasePlayer* pPlayer)'),
	('NetworkMessage ', 'MESSAGE_BEGIN '),
	('NetworkMessages::WeapPickup', 'gmsgWeapPickup'),
	('NetworkMessages::SVC_TEMPENTITY', 'SVC_TEMPENTITY'),
	('m_pPlayer.m_rgAmmo(', 'm_pPlayer->rgAmmo('),
	('.WriteLong', '.WRITE_LONG'),
	('.WriteCoord', '.WRITE_COORD'),
	('.WriteByte', '.WRITE_BYTE'),
	('.WriteShort', '.WRITE_SHORT'),
	('.End()', '.MESSAGE_END()'),
	('g_EngineFuncs.', 'g_engfuncs.pfn'),
	('m_pPlayer.', 'm_pPlayer->'),
	(': ScriptBasePlayerWeaponEntity', ': public CBasePlayerWeapon'),
	('& in ', '&'),
	('& out ', '&'),
	('& inout ', '&'),
	('EHandle ', 'EHANDLE'),
	('this.', 'this->'),
	('RemoveEntity', 'UTIL_Remove'),
]

def convert_script(fpath):
	print("Convert %s" % fpath)
			
	with codecs.open(fpath, 'r+', 'utf-8') as file:
		text = file.read()
		
		for replacement in terms:
			text = text.replace(replacement[0], replacement[1])
		
		file.seek(0)
		file.write(text)
		file.truncate()

if os.path.isdir(path):
	for root, dirs, files in os.walk(path):
		for file in files:
			if file.endswith('.as'):
				fpath = os.path.join(root, file)
				convert_script(fpath)
				os.rename(fpath, fpath.replace(".as", ".cpp"))
else:
	convert_script(path)