# 1) Place models using paths relative to the game dir (models/*)
# 2) Edit the merge_dirs below to select which models to merge
# 3) Edit the merged model path. This is ripented into the bsp files in the maps/ folder

import os, shutil, subprocess, struct
from collections import OrderedDict

ripent_path = 'ripent'
merged_model_path = "models/crystal_mission/props_v1.mdl"

def merge_props():
	global merged_model_path
	
	out_name = merged_model_path
	
	merge_dirs = [
		'models/crystal_mission/',
	]

	props = []
	for dir in merge_dirs:
		props += [dir + x for x in os.listdir(dir)]
	#props = props[:16]

	print("Copy " + props[0].strip())
	shutil.copy(props[0].strip(), out_name)

	mdl_to_body = {}
	mdl_to_body[props[0]] = 0

	bodyidx = 1

	for prop in props[1:]:
		prop = prop.strip()
		
		if not prop.endswith(".mdl"):
			continue
		
		mdlguy_command = ['modelguy', 'mergeprop', out_name, prop, out_name]
		
		if subprocess.run(mdlguy_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE).returncode:
			print("FAILED TO MERGE PROP %s" % prop)
			subprocess.run(mdlguy_command)
		else:
			print("Merged %s as submodel %d" % (prop, bodyidx))
			mdl_to_body[prop] = bodyidx
			bodyidx += 1
			
	with open("prop_bodies.txt", "w") as f:
		for key, val in mdl_to_body.items():
			f.write("%d=%s\n" % (val, key))

def parse_keyvalue(line):
	if line.find("//") != -1:
		line = line[:line.find("//")]
		
	quotes = [idx for idx, c in enumerate(line) if c == '"']
	
	if len(quotes) < 4:
		return None
	
	key   = line[quotes[0]+1 : quotes[1]]
	value = line[quotes[2]+1 : quotes[3]]
	
	return (key, value)

def parse_ents(path, ent_text):
	ents = []

	lineNum = 0
	lastBracket = -1
	ent = None
	
	for line in ent_text.splitlines():
		lineNum += 1
		
		if len(line) < 1 or line[0] == '\n':
			continue
			
		if line[0] == '{':
			if lastBracket == 0:
				print("\n%s.bsp ent data (line %d): Unexpected '{'\n" % (path, lineNum));
				continue
			lastBracket = 0

			ent = OrderedDict()

		elif line[0] == '}':
			if lastBracket == 1:
				print("\n%s.bsp ent data (line %d): Unexpected '}'\n" % (path, lineNum));
			lastBracket = 1

			if ent == None:
				continue

			ents.append(ent)
			ent = None

			# a new ent can start on the same line as the previous one ends
			if line.find("{") != -1:
				ent = OrderedDict()
				lastBracket = 0

		elif lastBracket == 0 and ent != None: # currently defining an entity
			keyvalue = parse_keyvalue(line)
			if keyvalue:
				ent[keyvalue[0]] = keyvalue[1]
	
	return ents

def load_entities(bsp_path):
	with open(bsp_path, mode='rb') as f:
		bytes = f.read()
		version = struct.unpack("i", bytes[:4])
		
		offset = struct.unpack("i", bytes[4:4+4])[0]
		length = struct.unpack("i", bytes[8:8+4])[0]
		
		ent_text = bytes[offset:offset+length].decode("utf-8", "ignore")
		
		return parse_ents(bsp_path, ent_text)
	
	print("\nFailed to open %s" % bsp_path)
	return None

def get_all_maps(maps_dir):
	all_maps = []
	
	for file in os.listdir(maps_dir):
		if not file.lower().endswith('.bsp'):
			continue
		if '@' in file:
			continue # ignore old/alternate versions of maps (w00tguy's scmapdb content pool)
			
		all_maps.append(file)
		
	return sorted(all_maps, key=lambda v: v.upper())

def apply_merged_model(all_ents, merged_model):
	any_changes = False
	
	mdl_to_body = {}
	for line in open('prop_bodies.txt', 'r').readlines():
		line = line.strip()
		if not line:
			continue
		parts = line.split("=")
		if len(parts) != 2:
			continue
		mdl_to_body[parts[1]] = parts[0]
	
	for ent in all_ents:
		model = ent.get('model', '').lower()
		
		if model in mdl_to_body:
			ent['model'] = merged_model
			ent['body'] = mdl_to_body[model]
			any_changes = True
			
	return any_changes

def ripent_maps(merged_model):
	maps_dir = "maps"
	
	all_maps = get_all_maps(maps_dir)

	for idx, map_name in enumerate(all_maps):
		map_path = os.path.join(maps_dir, map_name)
	
		all_ents = load_entities(map_path)
		
		if apply_merged_model(all_ents, merged_model):
			# ripent the map
			ent_file = map_path.replace(".bsp", ".ent")
			
			with open(ent_file, 'w', newline='\n', encoding='utf-8',) as file:
				for ent in all_ents:
					if ent.get('classname', ''):
						# keep classname last
						cname = ent['classname']
						del ent['classname']
						ent['classname'] = cname
				
					file.write('{\n')
					
					for key,value in ent.items():
						file.write('"%s" "%s"\n' % (key, value))
					
					file.write('}\n')
			
			ripent_command = [ripent_path, '-import', map_path]
			print(' '.join(ripent_command))
			subprocess.run(ripent_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
			
			os.remove(ent_file)

merge_props()

print("\nFinished merge. The next step will ripent the maps to make use of this merged model.")
os.system("pause")

ripent_maps(merged_model_path)
