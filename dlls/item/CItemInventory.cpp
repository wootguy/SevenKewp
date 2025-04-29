#include "CItemInventory.h"
#include "util.h"
#include "CWeaponInventory.h"

LINK_ENTITY_TO_CLASS(item_inventory, CItemInventory)

void CItemInventory::Spawn(void) {
	Precache();
	SetItemModel();

	CItem::Spawn();

	SetThink(&CItemInventory::ItemThink);
	pev->nextthink = gpGlobals->time + 0.1f;
}

void CItemInventory::Precache(void)
{
	m_defaultModel = "models/w_security.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_SOUND("items/gunpickup2.wav");
	PRECACHE_SOUND("tfc/items/itembk2.wav");

	AddPrecacheWeapon("weapon_inventory");
}

void CItemInventory::KeyValue(KeyValueData* pkvd) {
	if (FStrEq(pkvd->szKeyName, "item_name"))
	{
		m_item_name = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group"))
	{
		m_item_group = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "display_name"))
	{
		m_display_name = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "description"))
	{
		m_description = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_icon"))
	{
		m_icon = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "collect_limit"))
	{
		m_collect_limit = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "weight"))
	{
		m_weight = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "filter_targetnames"))
	{
		m_filter_targetnames = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "filter_classnames"))
	{
		m_filter_classnames = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "filter_teams"))
	{
		m_filter_teams = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "filter_npc_classifications"))
	{
		m_filter_npc_classifications = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_name_required"))
	{
		m_item_name_required = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_required"))
	{
		m_item_group_required = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_required"))
	{
		m_item_group_required = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_required_num"))
	{
		m_item_group_required_num = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_name_moved"))
	{
		m_item_name_moved = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_name_canthave"))
	{
		m_item_name_canthave = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_canthave"))
	{
		m_item_group_canthave = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_group_canthave_num"))
	{
		m_item_group_canthave_num = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "item_name_not_moved"))
	{
		m_item_name_not_moved = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holder_can_activate"))
	{
		m_holder_can_activate = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "activate_limit"))
	{
		m_activate_limit = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "return_timelimit"))
	{
		m_return_timelimit = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "return_delay_respawn"))
	{
		m_return_delay_respawn = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holder_timelimit"))
	{
		m_holder_timelimit = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holder_time_activate_wait"))
	{
		m_holder_time_activate_wait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holder_time_wearout"))
	{
		m_holder_time_wearout = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holder_can_drop"))
	{
		m_holder_can_drop = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holder_keep_on_death"))
	{
		m_holder_keep_on_death = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holder_keep_on_respawn"))
		{
		m_holder_keep_on_respawn = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holder_timelimit_wait_until_activated"))
	{
		m_holder_timelimit_wait_until_activated = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "carried_hidden"))
	{
		m_carried_hidden = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "carried_skin"))
	{
		m_carried_skin = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "carried_body"))
	{
		m_carried_body = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "carried_sequencename"))
	{
		m_carried_sequencename = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "carried_sequence"))
	{
		m_carried_sequence = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effects_wait_until_activated"))
	{
		m_effects_wait_until_activated = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effects_permanent"))
		{
		m_effects_permanent = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_glow"))
	{
		UTIL_StringToVector(m_effect_glow, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_block_weapons"))
	{
		m_effect_block_weapons = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_invulnerable"))
	{
		m_effect_invulnerable = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_invisible"))
	{
		m_effect_invisible = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_nonsolid"))
	{
		m_effect_nonsolid = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_respiration"))
	{
		m_effect_respiration = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_friction"))
	{
		m_effect_friction = atof(pkvd->szValue) / 100.0f;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_gravity"))
	{
		m_effect_gravity = atof(pkvd->szValue) / 100.0f;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_speed"))
	{
		m_effect_speed = atof(pkvd->szValue) / 100.0f;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_damage"))
	{
		m_effect_damage = atof(pkvd->szValue) / 100.0f;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_collect"))
	{
		m_target_on_collect.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_collect_team"))
	{
		m_target_on_collect.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_collect_other"))
	{
		m_target_on_collect.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_collect"))
	{
		m_target_cant_collect.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_collect_team"))
	{
		m_target_cant_collect.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_collect_other"))
	{
		m_target_cant_collect.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_drop"))
	{
		m_target_on_drop.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_drop_team"))
	{
		m_target_on_drop.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_drop_other"))
	{
		m_target_on_drop.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_drop"))
	{
		m_target_cant_drop.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_drop_team"))
	{
		m_target_cant_drop.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_drop_other"))
	{
		m_target_cant_drop.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_activate"))
	{
		m_target_on_activate.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_activate_team"))
	{
		m_target_on_activate.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_activate_other"))
	{
		m_target_on_activate.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_activate"))
	{
		m_target_cant_activate.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_activate_team"))
	{
		m_target_cant_activate.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_cant_activate_other"))
	{
		m_target_cant_activate.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_use"))
	{
		m_target_on_use.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_use_team"))
	{
		m_target_on_use.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_use_other"))
	{
		m_target_on_use.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_wearing_out"))
	{
		m_target_on_wearing_out.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_wearing_out_team"))
	{
		m_target_on_wearing_out.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_wearing_out_other"))
	{
		m_target_on_wearing_out.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_return"))
	{
		m_target_on_return.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_return_team"))
	{
		m_target_on_return.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_return_other"))
	{
		m_target_on_return.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_materialize"))
	{
		m_target_on_materialize.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_materialize_team"))
	{
		m_target_on_materialize.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_materialize_other"))
	{
		m_target_on_materialize.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_destroy"))
	{
		m_target_on_destroy.activatorTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_destroy_team"))
	{
		m_target_on_destroy.teamTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_on_destroy_other"))
	{
		m_target_on_destroy.othersTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CItem::KeyValue(pkvd);
	}
}

bool CItemInventory::StringInList(string_t str, string_t list) {
	if (!str || !list) {
		return false;
	}

	std::vector<std::string> listItems = splitString(STRING(str), ";");
	
	for (const std::string& item : listItems) {
		if (!strcmp(item.c_str(), STRING(str))) {
			return true;
		}
	}
	
	return false;
}

void CItemInventory::ItemUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pCaller && !pCaller->IsMonster()) {
		FireInvTargets(pActivator, m_target_on_use);
	}

	CItem::ItemUse(pActivator, pCaller, useType, value);
}

bool CItemInventory::CanCollect(CBaseMonster* pPlayer, const char** errorMsg) {
	CBasePlayer* plr = pPlayer->IsPlayer() ? (CBasePlayer*)pPlayer : NULL;

	if (plr && plr->m_nextItemPickups[WEAPON_INVENTORY] > gpGlobals->time) {
		return FALSE;
	}

	if (m_filter_targetnames && !StringInList(pPlayer->pev->targetname, m_filter_targetnames)) {
		*errorMsg = UTIL_VarArgs("You can't pick up this item.");
		return false;
	}
	if (m_filter_classnames) {
		ALERT(at_console, "Inventory filter_classnames not implemented\n");
	}
	if (m_filter_teams) {
		ALERT(at_console, "Inventory filter_teams not implemented: %s\n", STRING(m_filter_teams));
	}
	if (m_filter_npc_classifications) {
		ALERT(at_console, "Inventory filter_npc_classifications not implemented\n");
	}

	int numHeld = 0;
	int heldWeight = 0;
	bool hasRequiredItem = false;
	int requiredItemGroupCount = 0;
	int forbidItemGroupCount = 0;
	const char* forbidItemName = "unknown_item";

	CItemInventory* item = pPlayer->m_inventory ? pPlayer->m_inventory.GetEntity()->MyInventoryPointer() : NULL;

	while (item) {
		if (item->m_item_name) {
			if (m_item_name && !strcmp(STRING(m_item_name), STRING(item->m_item_name))) {
				numHeld++;
			}

			if (m_item_name_required && !strcmp(STRING(m_item_name_required), STRING(item->m_item_name))) {
				hasRequiredItem = true;
			}

			if (m_item_name_canthave && !strcmp(STRING(m_item_name_canthave), STRING(item->m_item_name))) {
				*errorMsg = UTIL_VarArgs("You can't pick up this item\nwhile '%s' is held.", item->DisplayName());
				return FALSE;
			}
		}

		if (item->m_item_group) {
			if (m_item_group_required && !strcmp(STRING(m_item_group_required), STRING(item->m_item_group))) {
				requiredItemGroupCount++;
			}

			if (m_item_group_canthave && !strcmp(STRING(m_item_group_canthave), STRING(item->m_item_group))) {
				forbidItemName = item->DisplayName();
				forbidItemGroupCount++;
			}
		}

		heldWeight += item->m_weight;

		item = item->m_pNext ? item->m_pNext.GetEntity()->MyInventoryPointer() : NULL;
	}

	if (m_collect_limit != 0 && numHeld > m_collect_limit) {
		*errorMsg = UTIL_VarArgs("You can't pick up more than %d of this item.\n", m_collect_limit);
		return false;
	}
	if (heldWeight > 100) {
		*errorMsg = UTIL_VarArgs("You can't pick up this item.\nDrop your other item(s) to reduce carried weight.",
			m_collect_limit);
		return false;
	}
	if (m_item_name_required && hasRequiredItem) {
		*errorMsg = "You can't pick up this item.\nYou lack a required item.";
		return false;
	}
	if (m_item_group_required && requiredItemGroupCount < m_item_group_required_num) {
		int itemNeed = m_item_group_required_num - requiredItemGroupCount;

		if (itemNeed == 1) {
			*errorMsg = "You can't pick up this item.\nYou lack a required item.";
		}
		else {
			*errorMsg = UTIL_VarArgs("You can't pick up this item.\nYou lack %d required items.", itemNeed);
		}
		return false;
	}
	if (m_item_group_canthave && forbidItemGroupCount >= m_item_group_canthave_num) {
		*errorMsg = UTIL_VarArgs("You can't pick up this item\nwhile '%s' is held.", forbidItemName);
		return false;
	}

	if (m_item_name_moved || m_item_name_not_moved) {
		bool requiredItemMoved = false;

		edict_t* edicts = ENT(0);

		for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++) {
			if (edicts[i].free)
				continue;

			CBaseEntity* ent = (CBaseEntity*)GET_PRIVATE(&edicts[i]);
			item = ent ? ent->MyInventoryPointer() : NULL;

			if (!item) {
				continue;
			}

			if (m_item_name_moved && item->m_is_moved) {
				*errorMsg = UTIL_VarArgs("You can't pick up this item\nwhile a '%s' has been picked up by anyone.",
					item->DisplayName());
				return false;
			}

			if (m_item_name_not_moved && item->m_is_moved) {
				requiredItemMoved = true;
			}
		}

		if (m_item_name_not_moved && !requiredItemMoved) {
			*errorMsg = "You can't pick up this item\nunless another item has been picked up.";
			return false;
		}
	}

	return true;
}

void CItemInventory::ItemTouch(CBaseEntity* pOther) {
	if (pev->movetype == MOVETYPE_BOUNCE && pOther->IsBSPModel()) {
		if (pev->velocity.Length() > 100) {
			pev->velocity = pev->velocity * 0.5f;
			if (RANDOM_LONG(0, 1)) {
				pev->avelocity.x *= -1;
				pev->avelocity.z *= -1;
			}
		}
		else {
			pev->movetype = MOVETYPE_TOSS;
			pev->avelocity = Vector(0, 0, 0);
		}

		int channel = (m_lastSoundChannel++ % 2) == 1 ? CHAN_VOICE : CHAN_ITEM;
		EMIT_SOUND_DYN(ENT(pev), channel, "items/weapondrop1.wav", 0.7f, ATTN_IDLE, 0, RANDOM_LONG(90, 110));

		pev->angles.x = 0;
		pev->angles.z = 0;
	}

	CBaseMonster* mon = pOther->MyMonsterPointer();
	CBasePlayer* plr = pOther->IsPlayer() ? (CBasePlayer*)pOther : NULL;

	if (!plr) {
		return;
	}

	const char* errorMsg = "";

	if (CanCollect(mon, &errorMsg)) {
		Attach(mon);
		ApplyModelProperties(true);

		if (plr && !plr->HasNamedPlayerItem("weapon_inventory")) {
			plr->GiveNamedItem("weapon_inventory");
		}

		FireInvTargets(pOther, m_target_on_collect);
	}
	else {
		FireInvTargets(pOther, m_target_cant_collect);

		if (plr) {
			plr->ShowInteractMessage(errorMsg);
		}
	}
}

int	CItemInventory::ObjectCaps(void) {
	return m_hHolder ? CBaseEntity::ObjectCaps() : CItem::ObjectCaps();
}

int CItemInventory::ActivateItem() {
	CBaseMonster* carrier = m_hHolder ? m_hHolder.GetEntity()->MyMonsterPointer() : NULL;

	if (!carrier || !m_holder_can_activate) {
		FireInvTargets(carrier, m_target_cant_activate);
		return INV_ERROR_CANT_ACTIVATE_EVER;
	}

	if (m_is_active) {
		FireInvTargets(carrier, m_target_cant_activate);
		return INV_ERROR_CANT_ACTIVATE_ACTIVE;
	}

	if (m_activate_limit && m_activation_count >= m_activate_limit) {
		FireInvTargets(carrier, m_target_cant_activate);
		return INV_ERROR_CANT_ACTIVATE_LIMIT;
	}

	float now = g_engfuncs.pfnTime();
	if (now - m_last_activation < m_holder_time_activate_wait) {
		FireInvTargets(carrier, m_target_cant_activate);
		return INV_ERROR_CANT_ACTIVATE_COOLDOWN;
	}
	m_last_activation = now;
	m_is_active = true;
	carrier->ApplyEffects();

	if (m_holder_timelimit_wait_until_activated && m_holder_timelimit) {
		m_drop_time = gpGlobals->time + m_holder_timelimit;
	}

	m_activation_count++;

	FireInvTargets(carrier, m_target_on_activate);

	return INV_ERROR_ACTIVATED;
}

void CItemInventory::ApplyModelProperties(bool carriedNotDropped) {
	if (carriedNotDropped) {
		pev->skin = m_carried_skin;
		pev->body = m_carried_body;

		if (m_carried_sequence >= 0) {
			pev->sequence = m_carried_sequence;
		}
		else if (m_carried_sequencename) {
			pev->sequence = LookupSequence(STRING(m_carried_sequencename));
		}
		else {
			pev->sequence = 0;
		}
	}
	else {
		pev->skin = m_return_skin;
		pev->body = m_return_body;

		if (m_return_sequence >= 0) {
			pev->sequence = m_return_sequence;
		}
		else if (m_sequence_name) {
			pev->sequence = LookupSequence(STRING(m_sequence_name));
		}
		else {
			pev->sequence = 0;
		}
	}

	ResetSequenceInfo();
}

void CItemInventory::Attach(CBaseMonster* mon) {
	if (!mon) {
		return;
	}

	if (!mon->m_inventory) {
		mon->m_inventory = this;
	}
	else {
		CItemInventory* item = mon->m_inventory.GetEntity()->MyInventoryPointer();

		while (item->m_pNext) {
			item = item->m_pNext.GetEntity()->MyInventoryPointer();
		}

		item->m_pNext = this;
	}

	if (!m_is_moved) {
		// collected for the first time, or after returning
		m_return_pos = pev->origin;
		m_return_angles = pev->angles;
		m_return_move_type = pev->movetype;
		m_return_skin = pev->skin;
		m_return_body = pev->body;
		m_return_sequence = pev->sequence;
	}

	m_pNext = NULL;
	m_hHolder = mon;
	m_is_moved = true;
	m_return_time = 0;
	SetTouch(NULL);
	pev->solid = SOLID_NOT;

	if (!m_holder_timelimit_wait_until_activated && m_holder_timelimit) {
		m_drop_time = gpGlobals->time + m_holder_timelimit;
	}

	m_wearout_time = gpGlobals->time + V_max(0, m_holder_timelimit - m_holder_time_wearout);

	if (mon->IsPlayer() && mon->IsAlive()) {
		CBasePlayer* pPlayer = (CBasePlayer*)mon;
		if (!pPlayer->HasNamedPlayerItem("weapon_inventory")) {
			pPlayer->GiveNamedItem("weapon_inventory");
		}
		else {
			// always play pickup effect so player knows they got something
			MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
			WRITE_BYTE(WEAPON_INVENTORY);
			MESSAGE_END();

			EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
		}
	}

	mon->ApplyEffects();
}

void CItemInventory::Detach(bool fireDropTrigger) {
	CBaseMonster* carrier = m_hHolder ? m_hHolder.GetEntity()->MyMonsterPointer() : NULL;

	if (!carrier) {
		return;
	}

	if (fireDropTrigger)
		FireInvTargets(carrier, m_target_on_drop);

	pev->aiment = NULL;
	pev->movetype = MOVETYPE_BOUNCE;
	pev->avelocity = Vector(0, 256, 256);
	pev->angles = g_vecZero;
	pev->effects &= ~EF_NODRAW;

	MAKE_VECTORS(carrier->pev->v_angle);
	pev->solid = SOLID_TRIGGER;
	pev->velocity = gpGlobals->v_forward * 400;

	UTIL_SetOrigin(pev, carrier->GetGunPosition());

	if (!(pev->spawnflags & SF_ITEM_USE_ONLY))
		SetTouch(&CItem::ItemTouch);

	ApplyModelProperties(false);
	m_wearout_time = 0;
	m_drop_time = 0;
	m_is_active = false;
	m_is_viewing = false;
	if (m_return_timelimit >= 0)
		m_return_time = gpGlobals->time + m_return_timelimit;

	if (carrier->m_inventory == this) {
		carrier->m_inventory = m_pNext;
		m_pNext = NULL;
		m_hHolder = NULL;
	}
	else if (carrier->m_inventory) {
		CItemInventory* item = carrier->m_inventory.GetEntity()->MyInventoryPointer();

		while (item) {
			if (item->m_pNext.GetEntity() == this) {
				item->m_pNext = m_pNext;
				m_pNext = NULL;
				m_hHolder = NULL;
				break;
			}

			item = item->m_pNext ? item->m_pNext.GetEntity()->MyInventoryPointer() : NULL;
		}
	}

	carrier->ApplyEffects();

	CBasePlayer* plr = carrier->IsPlayer() ? (CBasePlayer*)carrier : NULL;

	if (!carrier->m_inventory && plr) {
		CBasePlayerItem* item = plr->GetNamedPlayerItem("weapon_inventory");
		if (item) {
			if (item == plr->m_pActiveItem.GetEntity()) {
				g_pGameRules->GetNextBestWeapon(plr, item);
			}
			plr->RemovePlayerItem(item);
		}
	}

	if (plr)
		plr->m_nextItemPickups[WEAPON_INVENTORY] = gpGlobals->time + 3.0f;
}

void CItemInventory::ReturnToSpawnPosition() {
	CBaseMonster* carrier = m_hHolder ? m_hHolder.GetEntity()->MyMonsterPointer() : NULL;

	if (carrier) {
		Detach(true);
	}

	ApplyModelProperties(false);
	UTIL_SetOrigin(pev, m_return_pos);

	m_is_active = false;
	m_is_moved = false;
	m_last_activation = 0;
	m_activation_count = 0;
	m_return_time = 0;
	m_drop_time = 0;
	m_wearout_time = 0;
	pev->movetype = m_return_move_type;
	pev->angles = m_return_angles;
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;

	CBaseEntity* activator = carrier ? carrier : (CBaseEntity*)this;
	FireInvTargets(activator, m_target_on_return);
}

void CItemInventory::ItemThink() {
	CBaseMonster* carrier = m_hHolder ? m_hHolder.GetEntity()->MyMonsterPointer() : NULL;

	float thinkDelay = 0.1f;

	if (carrier) {
		pev->aiment = NULL;
		pev->movetype = MOVETYPE_NOCLIP;

		if (m_is_viewing != m_was_viewing) {
			// show dropped model properties in case it has an animation for hovering above the player
			// (will look very wrong if rotated)
			ApplyModelProperties(!m_is_viewing);
		}

		m_was_viewing = m_is_viewing;

		if (m_is_viewing) {
			Vector angles = carrier->pev->v_angle;
			angles.x += sinf(g_engfuncs.pfnTime() * 0.1f); // keep it moving so avelocity is smoothed

			MAKE_VECTORS(angles);

			Vector start = carrier->GetGunPosition();
			Vector ori = start + gpGlobals->v_forward * 64;
			TraceResult tr, tr2;
			TRACE_HULL(start, ori, dont_ignore_monsters, head_hull, carrier->edict(), &tr);

			if (tr.fStartSolid) {
				TRACE_LINE(start, ori, dont_ignore_monsters, carrier->edict(), &tr);
			}

			Vector dir = ori - start;
			Vector end = start + (V_max(0.2f, tr.flFraction) * dir.Length() * dir.Normalize());

			pev->avelocity = Vector(50, 200, 0);
			pev->velocity = (end - pev->origin) * gpGlobals->frametime * 4000;

			if ((end - pev->origin).Length() > 32) {
				UTIL_SetOrigin(pev, end);
			}

			pev->effects &= ~EF_NODRAW;

			thinkDelay = 0.0f;
		}
		else {
			pev->aiment = carrier->edict();
			pev->movetype = MOVETYPE_FOLLOW;
			pev->origin = carrier->pev->origin;

			if (m_carried_hidden) {
				pev->effects |= EF_NODRAW;
			}
		}
	}
	else if (m_is_moved && pev->movetype == MOVETYPE_NONE) {
		// the engine will set movetype to none if this think() was executed in the follow branch
		// because the aiment was removed after Detach()
		pev->movetype = MOVETYPE_BOUNCE;
	}

	if (m_waiting_to_materialize) {
		if (!(pev->spawnflags & SF_ITEM_USE_ONLY))
			SetTouch(&CItem::ItemTouch);
		pev->solid = SOLID_TRIGGER;
		pev->effects &= !EF_NODRAW;
		m_waiting_to_materialize = false;
		FireInvTargets(this, m_target_on_materialize);
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
	}

	if (m_wearout_time && m_wearout_time < gpGlobals->time) {
		m_wearout_time = 0;
		FireInvTargets(carrier, m_target_on_wearing_out);
	}

	if (m_drop_time && m_drop_time < gpGlobals->time) {
		Detach(true);
	}

	if (m_return_time && m_return_time < gpGlobals->time) {
		ReturnToSpawnPosition();
		EMIT_SOUND_DYN(edict(), CHAN_ITEM, "tfc/items/itembk2.wav", 1, ATTN_NORM, 0, 150);

		if (m_return_delay_respawn) {
			SetTouch(NULL);
			pev->solid = SOLID_NOT;
			pev->effects |= EF_NODRAW;
			thinkDelay = g_pGameRules->FlItemRespawnTime(this) - gpGlobals->time;
			m_waiting_to_materialize = true;
		}
	}

	pev->nextthink = gpGlobals->time + thinkDelay;
}

void CItemInventory::FireInvTargets(CBaseEntity* activator, InvTriggerTargets target) {
	if (target.activatorTarget)
		FireTargets(STRING(target.activatorTarget), activator, this, USE_TOGGLE, 0.0f);

	if (target.teamTarget) {
		ALERT(at_console, "Inventory team target triggers not implemented\n");
	}

	if (target.othersTarget) {
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			CBaseEntity* pPlayer = UTIL_PlayerByIndex(i);

			if (pPlayer && pPlayer != activator) {
				FireTargets(STRING(target.othersTarget), pPlayer, this, USE_TOGGLE, 0.0f);
			}
		}
	}
}

void CItemInventory::UpdateOnRemove(void) {
	Detach(false);
	FireInvTargets(this, m_target_on_destroy);
}

int CountAllItemsInGroups(const char* groupNames) {
	std::vector<std::string> names = splitString(groupNames, " ");
	edict_t* edicts = ENT(0);

	int count = 0;

	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++) {
		if (edicts[i].free)
			continue;

		CBaseEntity* ent = (CBaseEntity*)GET_PRIVATE(&edicts[i]);
		CItemInventory* item = ent ? ent->MyInventoryPointer() : NULL;

		if (item && item->m_item_group) {
			for (int k = 0; k < (int)names.size(); k++) {
				if (!strcmp(names[k].c_str(), STRING(item->m_item_group))) {
					count++;
				}
			}
		}
	}

	return count;
}