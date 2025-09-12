# This script creates the w_items model.
# Copy all the model files into the same folder as this script before running.
# Bump the version number on updates, and update the .gmr file too.

import os, shutil, subprocess

def merge_props():
	out_name = 'w_items_v3.mdl'

	props = [
		'w_9mmar.mdl',
		'w_9mmarclip.mdl',
		'w_9mmclip.mdl',
		'w_9mmhandgun.mdl',
		'w_357.mdl',
		'w_357ammobox.mdl',
		'w_argrenade.mdl',
		'w_battery.mdl',
		'w_bgrap.mdl',
		'w_chainammo.mdl',
		'w_crossbow.mdl',
		'w_crossbow_clip.mdl',
		'w_crowbar.mdl',
		'w_displacer.mdl',
		'w_egon.mdl',
		'w_gauss.mdl',
		'w_gaussammo.mdl',
		'w_grenade.mdl',
		'w_hgun.mdl',
		'w_longjump.mdl',
		'w_medkit.mdl',
		'w_pipe_wrench.mdl',
		'w_rpg.mdl',
		'w_rpgammo.mdl',
		'w_satchel.mdl',
		'w_security.mdl',
		'w_shotbox.mdl',
		'w_shotgun.mdl',
		'w_shotshell.mdl',
		'w_suit.mdl',
		'w_weaponbox.mdl',
		'grenade.mdl',
		'hvr.mdl',
		'rpgrocket.mdl',
		'spore.mdl',
		'shock_effect.mdl',
		'w_2uzis.mdl',
		'w_uzi.mdl',
		'w_uzi_clip.mdl',
		'w_saw.mdl',
		'w_saw_clip.mdl',
		'w_desert_eagle.mdl',
		'w_m40a1.mdl',
		'w_m40a1clip.mdl',
		'w_minigun.mdl',
		'w_knife.mdl',
		'w_m16.mdl',
		'w_pmedkit.mdl',
		'camera.mdl',
	]

	print("Copy " + props[0].strip())
	shutil.copy(props[0].strip(), out_name)

	mdl_to_body = {}
	mdl_to_body[props[0]] = 0

	bodyidx = 1

	for prop in props[1:]:
		prop = prop.strip()
		mdlguy_command = ['modelguy', 'mergeprop', out_name, prop, out_name]
		
		if subprocess.run(mdlguy_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE).returncode:
			print("FAILED TO MERGE PROP %s" % prop)
			subprocess.run(mdlguy_command)
		else:
			print("Merged %s as submodel %d" % (prop, bodyidx))
			mdl_to_body[prop] = bodyidx
			bodyidx += 1
			
merge_props()