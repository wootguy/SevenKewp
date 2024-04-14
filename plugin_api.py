import shutil, sys, os

if len(sys.argv) <= 1:
	print("Usage: plugin_api.py <output_folder>")
	print("This script will delete the output folder contents then fill it with new headers")
	sys.exit()

out_folder = sys.argv[1]

if os.path.exists(out_folder):
	shutil.rmtree(out_folder)
os.makedirs(out_folder)

copy_folders = [
	'common',
	'engine',
	'game_shared',
	'pm_shared',
	'public',
	'public/steam',
	'dlls',
	'dlls/env',
	'dlls/func',
	'dlls/triggers',
	'dlls/monster',
	'dlls/item',
	'dlls/path',
	'dlls/weapon'
]

for folder in copy_folders:
	for filename in os.listdir(folder):
		if filename.endswith('.h'):
			in_path = os.path.join(folder, filename)
			out_path_dir = os.path.join(out_folder, 'hlcoop', folder)
			if not os.path.exists(out_path_dir):
				os.makedirs(out_path_dir)
			shutil.copyfile(in_path, os.path.join(out_path_dir, filename))