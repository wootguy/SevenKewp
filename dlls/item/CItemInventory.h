#pragma once
#include "CItem.h"

struct InvTriggerTargets {
	string_t activatorTarget;	// target fired by the activator
	string_t teamTarget;		// target fired by all team members of the activator
	string_t othersTarget;		// target fired by all players except for the activator
};

// count all items in the world with the given group names (separated by spaces)
int CountAllItemsInGroups(const char* groupNames);

class EXPORT CItemInventory : public CItem
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	virtual void ItemUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	bool CanCollect(CBaseMonster* pPlayer, const char** errorMsg);
	virtual void ItemBounce(CBaseEntity* pOther);
	virtual void ItemTouch(CBaseEntity* pOther) override;
	virtual BOOL ShouldRespawn() { return FALSE; } // can only have one instance of itself in the world
	virtual int	ObjectCaps(void);
	virtual CItemInventory* MyInventoryPointer(void) { return this; }
	bool StringInList(string_t str, string_t list);
	virtual const char* DisplayName() { return m_display_name ? STRING(m_display_name) : "item_inventory"; }
	int ActivateItem();
	void ApplyModelProperties(bool carriedNotDropped);
	void ItemThink();
	void ReturnToSpawnPosition();
	void Attach(CBaseMonster* mon);
	void Detach(bool fireDropTrigger); // detach from the holding player
	void FireInvTargets(CBaseEntity* activator, InvTriggerTargets target);
	void UpdateOnRemove(void);

	string_t m_item_name;	// name referred to by inventory rule keys, not a targetname
	string_t m_item_group;	// name referred to by inventory rule keys, not a targetname
	
	// HUD display info
	string_t m_display_name;
	string_t m_description;
	string_t m_icon; // TODO: custom sprites for HUD (custom client needed)

	// item pick up rules
	int m_collect_limit;			// how many duplicates a player can carry
	int m_weight;					// the limit for all items in inventory is 100
	string_t m_filter_targetnames;	// only entities with the given targetname can pick up
	string_t m_filter_classnames;	// only entities with the given class can pick up
	string_t m_filter_teams;		// only entities in the given team can pick up (TODO: teams)
	string_t m_filter_npc_classifications; // only monsters with the given classes can pick up
	string_t m_item_name_required;	// required item needed before picking up this item
	string_t m_item_group_required;	// item from required group needed before picking up this item
	int m_item_group_required_num;	// count of items from group required before picking up this item
	string_t m_item_name_moved;		// allow pick up only if specified item hasn't moved or been picked up
	string_t m_item_name_canthave;	// forbid pick up if named item is held
	string_t m_item_group_canthave;	// forbid pick up if item from named group is held
	int m_item_group_canthave_num;	// count of items in canthave group that forbids pick up
	string_t m_item_name_not_moved;	// forbid pick up if the named item hasn't been moved or picked up yet

	// item activation rules
	bool m_holder_can_activate;		// player can activate the item from menu
	int m_activate_limit;			// limit self-activations by holding player
	float m_holder_time_activate_wait;	// time between activations while held

	// item carry rules
	float m_return_timelimit;			// time after dropping when the item returns to its starting position
	bool m_return_delay_respawn;		// wait for respawn after returning
	float m_holder_timelimit;			// max time item can be held before returning
	float m_holder_time_wearout;		// time before wearing out trigger is fired (subtracted from hold time)
	bool m_holder_can_drop;				// can be dropped manually
	bool m_holder_keep_on_death;
	bool m_holder_keep_on_respawn;
	bool m_holder_timelimit_wait_until_activated; // hold time doesn't start until activated

	// carried item properties
	bool m_carried_hidden;	// hide item from 3rd person view, otherwise show floating above head
	int m_carried_skin;		// model skin while carried
	int m_carried_body;		// model body while carried
	string_t m_carried_sequencename; // sequence to play while carried
	int m_carried_sequence;	// sequence to play while carried

	// applied player effects
	bool m_effects_wait_until_activated; // effects aren't applied until item activated
	bool m_effects_permanent;	// effects end after respawn, not when the item is dropped/lost
	Vector m_effect_glow;		// glow color. (0,0,0) = no glow
	bool m_effect_block_weapons; // prevent player from using weapons
	bool m_effect_invulnerable; // god mode
	bool m_effect_invisible;	// no target
	bool m_effect_nonsolid;
	float m_effect_respiration;	// additional/removed time for drowning
	float m_effect_friction;	// percent friction changed
	float m_effect_gravity;		// percent gravity changed
	float m_effect_speed;		// percent speed changed
	float m_effect_damage;		// player weapon damage percent changed

	InvTriggerTargets m_target_on_collect;
	InvTriggerTargets m_target_cant_collect;
	InvTriggerTargets m_target_on_drop;
	InvTriggerTargets m_target_cant_drop;
	InvTriggerTargets m_target_on_activate;
	InvTriggerTargets m_target_cant_activate;
	InvTriggerTargets m_target_on_use;
	InvTriggerTargets m_target_on_wearing_out;
	InvTriggerTargets m_target_on_return;
	InvTriggerTargets m_target_on_materialize;
	InvTriggerTargets m_target_on_destroy;

	// runtime state
	bool m_is_moved;	// true if the item was picked up or dropped (unset after returning)
	bool m_is_viewing;	// true if player is viewing the item on their "HUD"
	bool m_was_viewing;	// true if player was viewing the item last think
	EHANDLE m_hHolder;	// monster holding this item
	EHANDLE m_pNext;	// next item in the monster's inventory
	int m_activation_count;

	Vector m_return_pos;	// where the item returns to
	Vector m_return_angles;
	int m_return_move_type;
	int m_return_skin;
	int m_return_body;
	int m_return_sequence;
	bool m_is_active;
	float m_last_activation;
	float m_drop_time; // time when item will be dropped
	float m_return_time; // time when item will be forced back to its starting position. 0 = never
	float m_wearout_time;
	bool m_waiting_to_materialize;
	int m_lastSoundChannel;
};