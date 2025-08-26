# this script ports an SC 3.0 uzi model to hl co-op.
# Requires you to decompile with crowbar first, then compile after running the script
# You will likely need to edit bone indexes, frame offsets, and animation names below

import sys, os

def lower_left_arm(smd_path, start_frame, end_frame):
	global left_shoulder_bone_idx
	global left_elbow_bone_idx
	global anims_dir
	global arm_hide_offset
	
	newlines = open(os.path.join(anims_dir, smd_path)).readlines()
	
	writing_anim = False
	frame = 0
	
	with open(os.path.join(anims_dir, smd_path), "w") as f:
		for line in newlines:
			
			if line.strip().startswith('time'):
				writing_anim = True
				frame = int(line.split()[1])
				f.write(line)
				continue
			elif line.strip() == 'end':
				f.write(line)
				continue
			
			if not writing_anim:
				f.write(line)
				continue

			parts = line.split()
			
			boneid = int(parts[0])
			
			is_target_frame = True
			if start_frame != -1 and frame < start_frame:
				is_target_frame = False
			if end_frame != -1 and frame > end_frame:
				is_target_frame = False
			
			is_target_bone = boneid == left_shoulder_bone_idx or boneid == left_elbow_bone_idx
			
			if is_target_bone and is_target_frame:
				f.write('  %d  0.000000 %d.000000 0.000000 -0.000000 0.000000 0.000000\n' % (boneid, arm_hide_offset))
			else:
				f.write(line)

def extract_animation(smd_path, start_frame, end_frame, out_smd_path):
	global left_shoulder_bone_idx
	global left_elbow_bone_idx
	global anims_dir
	
	newlines = open(os.path.join(anims_dir, smd_path)).readlines()
	
	writing_anim = False
	frame = 0
	
	frames_written = False
	
	with open(os.path.join(anims_dir, out_smd_path), "w") as f:
		for line in newlines:
			
			if line.strip().startswith('time'):
				writing_anim = True
				frame = int(line.split()[1])
			elif line.strip() == 'end':
				f.write(line)
				continue
			
			if not writing_anim:
				f.write(line)
				continue
			
			is_target_frame = True
			if start_frame != -1 and frame < start_frame:
				is_target_frame = False
			if end_frame != -1 and frame > end_frame:
				is_target_frame = False
			
			if is_target_frame:
				f.write(line)
				frames_written = True
				
	if not frames_written:
		print("ERROR: No frames written for %s (bad start/end frames selected)\n" % out_smd_path)

def parse_sequences(qc_text):
    lines = [line.strip() for line in qc_text.splitlines() if line.strip()]
    sequences = []
    current = None

    for line in lines:
        if line.startswith("$sequence"):
            # Start new block
            name = line.split('"')[1]
            current = {
                "name": name,
                "model": None,
                "activity": None,
                "fps": None,
                "loop": False,
                "events": []
            }
        elif line.startswith("}"):
            # End block
            if current:
                sequences.append(current)
                current = None
        elif current is not None:
            # Inside a sequence block
            if line.startswith('"'):
                current["path"] = line.strip('"')
            elif line.lower().startswith("fps"):
                parts = line.split()
                current["fps"] = int(parts[1])
            elif line.lower().startswith("act_"):
                parts = line.split()
                current["activity"] = {"name": parts[0], "value": int(parts[1])}
            elif line.lower().startswith("loop"):
                current["loop"] = True
            elif line.startswith("{ event"):
                parts = line.strip("{} ").split()
                # format: event code frame "param"
                code = int(parts[1])
                frame = int(parts[2])
                param = line.split('"')[1] if '"' in line else None
                current["events"].append({
                    "code": code,
                    "frame": frame,
                    "param": param
                })

    return sequences

def find_seq(seqs, name):
	for idx, seq in enumerate(seqs):
		if seq["name"] == name:
			return idx
			
	return -1

def update_qc():
	global qc_path
	global new_sequences
	global anims_dir
	global delete_sound_events
	
	newlines = open(qc_path).readlines()
	
	seqs = parse_sequences(open(qc_path).read())
	
	new_seqs = []
	
	for seq_pair in new_sequences:
		old_name = seq_pair[0]
		new_name = seq_pair[1]
		
		idx = find_seq(seqs, old_name)
		
		if idx == -1:
			print("ERROR: Failed to find sequence: %s\n", old_name)
			continue
		
		dup_seq = seqs[idx].copy()
		
		if new_name in ['akimbo_holster', 'akimbo_deploy2']:
			dup_seq["path"] = anims_dir + "\\" + new_name
		
		dup_seq["name"] = new_name
		new_seqs.append(dup_seq)
		
	seqs = new_seqs
	
	with open(qc_path, "w") as f:
		writing_seq = False
		for line in newlines:
			if line.startswith('$sequence'):
				writing_seq = True
			if writing_seq and line.startswith("}"):
				writing_seq = False
				continue
			if writing_seq:
				continue
			
			if line.startswith('$hbox'):
				continue # may reference deleted bones so don't include hitboxes (they're useless anyway for v models)
			
			f.write(line)
			
		for seq in seqs:
			f.write('$sequence "%s" {\n' % seq["name"])
			f.write('\t"%s"\n' % seq["path"])
			if seq["activity"]:
				f.write('\t%s %s\n' % (seq["activity"]["name"], seq["activity"]["value"]))
			f.write('\tfps %d\n' % seq["fps"])
			if seq["loop"]:
				f.write('\tloop\n')
				
			for event in seq["events"]:
				if delete_sound_events and event["code"] == 5004:
					continue # no sound events (handled in code)
				f.write('\t{ event %d %d "%s" }\n' % (event["code"], event["frame"], event["param"]))
				
			f.write('}\n')

def parse_smd(smdpath):
	newlines = open(smdpath).readlines()
	
	WRITE_NODES = 0
	WRITE_SKEL = 1
	WRITE_TRIS = 2

	parse_mode = WRITE_NODES
	
	smd = {
		"bones": [],
		"tris": []
	}
	
	last_tex = ''
	verts = []
	
	for line in newlines:
		if line.strip() == 'nodes':
			parse_mode = WRITE_NODES
			continue
		elif line.strip() == 'time 0':
			parse_mode = WRITE_SKEL
			continue
		elif line.strip() == 'triangles':
			parse_mode = WRITE_TRIS
			continue
		elif line in ['end', 'skeleton', 'version 1']:
			continue
		
		if line[0] != ' ':
			if parse_mode == WRITE_TRIS:
				last_tex = line.strip()
			continue

		parts = line.split()
		
		if parse_mode == WRITE_NODES:
			boneid = int(parts[0])
			parentid = int(parts[-1])
			name = " ".join(parts[1:-1])
			
			bone = {"id": boneid, "parentid": parentid, "name": name}
			smd["bones"].append(bone)
		
		elif parse_mode == WRITE_SKEL:
			boneid = int(parts[0])
			dat = " ".join(parts[1:])
			
			smd["bones"][boneid]["dat"] = dat
			
		elif parse_mode == WRITE_TRIS:
			boneid = int(parts[0])
			dat = " ".join(parts[1:])
			
			verts.append({"bone": boneid, "dat": dat})
			
			if len(verts) == 3:
				smd["tris"].append({'tex': last_tex, 'verts': verts})
				verts = []
				last_tex = ''
			
	return smd

def delete_left_uzi_mesh():
	global left_uzi_ref_name
	global left_uzi_bone_idx
	
	smdpath = left_uzi_ref_name + '.smd'
	newlines = open(smdpath).readlines()
	
	writing_tris = False
	frame = 0
	
	smd = parse_smd(smdpath)
	
	delete_mesh_bones = [left_uzi_bone_idx]
	
	for bone in smd["bones"]:
		if bone["parentid"] in delete_mesh_bones:
			delete_mesh_bones.append(bone["id"])
	
	with open(smdpath, "w") as f:		
		for line in newlines:
			if line.strip() == 'triangles':
				break
			f.write(line)
		
		f.write("triangles\n")
		
		for tri in smd["tris"]:
			should_write_tri = True
			
			for vert in tri["verts"]:
				if vert["bone"] in delete_mesh_bones:
					should_write_tri = False
					break
					
			if should_write_tri:
				f.write("%s\n" % tri["tex"])
				for vert in tri["verts"]:
					f.write("  %d %s\n" % (vert["bone"], vert["dat"]))
					
		f.write("end\n")
					

#
# Edit below for your specific model
#

left_shoulder_bone_idx = 8
left_elbow_bone_idx = 9
arm_hide_offset = 20
delete_sound_events = True		# set to True if the sounds aren't duplicates of the default sounds

left_uzi_bone_idx = 26		# Delete mesh from this bone and all children (left uzi)
left_uzi_ref_name = 'v_uzi_uzi_reference2'

mdl_name = 'v_uzi'

anims_dir = mdl_name + '_anims'
qc_path = mdl_name + '.qc'


# maps old sequence name to new name.
new_sequences = [
	('idle_3', 'idle_1'),
	('idle_2', 'idle_2'),
	('deploy', 'deploy'),
	('deploy2', 'deploy2'),
	('shoot', 'shoot'),
	('reload', 'reload'),
	('akimbo_idle', 'akimbo_idle'),
	('akimbo_pull', 'akimbo_pull'),
	('akimbo_deploy', 'akimbo_deploy'),
	('akimbo_reload_left', 'akimbo_deploy2'),
	('akimbo_reload_left', 'akimbo_holster'),
	('akimbo_reload_right', 'akimbo_reload_right'),
	('akimbo_fire_right1', 'akimbo_fire_right1'),
]

# anims where the left hand should be lowered for the entire duration
for anim in ['akimbo_deploy', 'akimbo_idle', 'akimbo_fire_right1', 'akimbo_reload_left']:
	lower_left_arm('%s.smd' % anim, -1, -1)
	
# lowered for end of the animation
lower_left_arm('akimbo_pull.smd', 100, -1)

# lowered for beginning and end of animation
lower_left_arm('akimbo_reload_right.smd', -1, 10)
lower_left_arm('akimbo_reload_right.smd', 105, -1)

# akimbo holster/deploy extracted from the reload anim
extract_animation('akimbo_reload_left.smd', -1, 10, 'akimbo_holster.smd')
extract_animation('akimbo_reload_left.smd', 124, -1, 'akimbo_deploy2.smd')

delete_left_uzi_mesh()

update_qc()
