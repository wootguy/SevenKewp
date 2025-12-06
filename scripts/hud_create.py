from PIL import Image
from pathlib import Path
import os, sys, struct

def load_spr_first_frame(path):
	with open(path, "rb") as f:
		header_fmt = "<4sIII f III f I H"
		header_size = struct.calcsize(header_fmt)
		data = f.read(header_size)

		(
			ident, version, mode, spr_format, radius,
			width, height, frames, beam_len, sync_type, palette_sz
		) = struct.unpack(header_fmt, data)

		if ident != b"IDSP":
			raise ValueError(f"{path} is not a valid SPR file")

		# palette (RGB triplets)
		palette = []
		for _ in range(palette_sz):
			r, g, b = struct.unpack("BBB", f.read(3))
			palette.append((r, g, b))

		# frame header
		frame_header_fmt = "<IiiII"
		fhd = f.read(struct.calcsize(frame_header_fmt))
		group, offx, offy, fw, fh = struct.unpack(frame_header_fmt, fhd)

		# pixel data
		pixels = f.read(fw * fh)

		return {
			"width": fw,
			"height": fh,
			"pixels": pixels,
			"palette": palette
		}

def spr_frame_to_image(frame):
	img = Image.frombytes("P", (frame["width"], frame["height"]), frame["pixels"])
	pal = []
	for r, g, b in frame["palette"]:
		pal += [r, g, b]
	img.putpalette(pal)
	return img

def process_definition_file(txt_path):
	with open(txt_path, "r") as f:
		lines = f.read().splitlines()

	# Skip first line (count)
	entries = lines[1:]

	for line in entries:
		if not line.strip() or line.startswith("//"):
			continue

		parts = line.split()
		if len(parts) < 7:
			continue

		out_name = parts[0]
		resolution = parts[1]
		spr_rel = parts[2]
		x, y, w, h = map(int, parts[3:7])

		# only extract 640 entries
		if resolution != "640":
			continue

		spr_path = "sprites/" + spr_rel + ".spr"

		print(f"Loading {spr_path} ...")

		frame = load_spr_first_frame(spr_path)
		
		if out_name in ["crosshair", "autoaim"]:
			frame["palette"][255] = (0,0,255)
		
		img = spr_frame_to_image(frame)

		# crop the region from the first frame
		cropped = img.crop((x, y, x + w, y + h))

		out_file = f"{out_name}.bmp"
		cropped.save(out_file)

		print(f"Saved {out_file}")

def make_hud_spr(bmp, output_file):
	width = bmp["width"]
	height = bmp["height"]
	palette = bmp["palette"]
	pixels = bmp["pixels"]
	
	VP_PARALLEL_UPRIGHT = 0
	FACING_UPRIGHT = 1
	VP_PARALLEL = 2
	ORIENTED = 3
	VP_PARALLEL_ORIENTED = 4
	
	SPR_NORMAL = 0
	SPR_ADDITIVE = 1
	SPR_INDEXALPHA = 2
	SPR_ALPHATEST = 3

	header_struct = struct.pack(
		"<4sIII f II I f I H",
		b"IDSP",
		2,
		VP_PARALLEL,
		SPR_ALPHATEST,
		1.0,		# radius
		width,
		height,
		1,			# frames
		1.0,		# beamLength
		0,			# syncType
		256			# paletteSz
	)

	fheader = struct.pack(
		"<IiiII",
		0,		 # group
		0, 0,	 # x, y
		width,
		height
	)

	with open(output_file, "wb") as f:
		f.write(header_struct)
		f.write(palette)		# already raw bytes
		f.write(fheader)
		f.write(pixels)			# already raw bytes

def save_img_alphatest(canvas, out_name):
	canvas = canvas.quantize(colors=256, method=2)
	
	
	pal = canvas.getpalette()
	dst = 255
	src = 255
	for i in range(256):
		r, g, b = pal[i*3:i*3+3]
		if (r, g, b) == (0, 0, 255):
			src = i
			break
			
	# swap blue color to index 255
	pal[src*3:src*3+3], pal[dst*3:dst*3+3] = pal[dst*3:dst*3+3], pal[src*3:src*3+3]
	canvas.putpalette(pal)
	
	# remap pixels to new blue
	data = list(canvas.getdata())
	newdata = [dst if p == src else p for p in data]
	canvas.putdata(newdata)
	
	canvas.save(out_name + ".bmp")
	
	bmpdat = {
		"width": canvas.width,
		"height": canvas.height,
		"palette": bytes(pal),
		"pixels": canvas.tobytes(),
	}
	make_hud_spr(bmpdat, out_name + ".spr")
	print("Wrote %s" % out_name + ".spr")

def compile_weapon_hud():
	if not os.path.exists("weapon.bmp"):
		print("weapon.bmp does not exist. Cannot create a HUD file without the weapon icon.")
		sys.exit()
	if not os.path.exists("weapon_s.bmp"):
		print("weapon_s.bmp does not exist. Cannot create a HUD file without the highlighted weapon icon.")
		sys.exit()

	new_img = Image.new("RGB", (512, 512), "blue")

	# weapon icons
	weapon = Image.open('weapon.bmp')
	weapon_s = Image.open('weapon_s.bmp')
	new_img.paste(weapon.resize((int(170 * 3), int(45 * 3)), Image.NEAREST), (0, 0))
	new_img.paste(weapon_s.resize((int(170 * 3), int(45 * 3)), Image.NEAREST), (0, 135))
	new_img.paste(weapon.resize((int(170 * 2), int(45 * 2)), Image.NEAREST), (0, 270))
	new_img.paste(weapon_s.resize((int(170 * 2), int(45 * 2)), Image.NEAREST), (0, 360))
	new_img.paste(weapon, (0, 450))
	new_img.paste(weapon_s, (170, 450))
	new_img.paste(weapon.convert("RGB").resize((80,20), Image.LANCZOS), (388, 462))
	new_img.paste(weapon_s.convert("RGB").resize((80,20), Image.LANCZOS), (388, 482))

	has_ammo = False
	has_ammo2 = False
	has_crosshair = False
	has_autoaim = False
	has_zoom = False
	zoom_size = 512

	# ammo icons
	if os.path.exists("ammo.bmp"):
		ammo = Image.open('ammo.bmp')
		new_img.paste(ammo.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (340, 270))
		new_img.paste(ammo.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (340, 414))
		new_img.paste(ammo, (484, 270))
		new_img.paste(ammo.convert("RGB").resize((18,18), Image.LANCZOS), (484, 366))
		has_ammo = True
	else:
		print("ammo.bmp does not exist. Primary ammo icons will not be added to the HUD.")

	if os.path.exists("ammo2.bmp"):
		ammo2 = Image.open('ammo2.bmp')
		new_img.paste(ammo2.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (412, 270))
		new_img.paste(ammo2.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (388, 414))
		new_img.paste(ammo2, (484, 294))
		new_img.paste(ammo2.convert("RGB").resize((18,18), Image.LANCZOS), (484, 384))
		has_ammo2 = True
	else:
		print("ammo2.bmp does not exist. Secondary ammo icons will not be added to the HUD.")

	# crosshair icons
	if os.path.exists("crosshair.bmp"):
		crosshair = Image.open('crosshair.bmp')
		new_img.paste(crosshair.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (340, 342))
		new_img.paste(crosshair.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (340, 462))
		new_img.paste(crosshair, (484, 318))
		has_crosshair = True
	else:
		print("crosshair.bmp does not exist. Crosshairs will not be added to the HUD.")

	# autoaim icons
	if os.path.exists("autoaim.bmp"):
		autoaim = Image.open('autoaim.bmp')
		new_img.paste(autoaim.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (412, 342))
		new_img.paste(autoaim.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (436, 414))
		new_img.paste(autoaim, (484, 342))
		new_img.paste(autoaim.convert("RGB").resize((18,18), Image.LANCZOS), (484, 402))
		has_autoaim = True
	else:
		print("autoaim.bmp does not exist. Autoaim crosshairs will not be added to the HUD.")

	if os.path.exists("zoom.bmp"):
		zoom = Image.open('zoom.bmp')
		zoom_size = zoom.width
		has_zoom = True
	else:
		print("zoom.bmp does not exist. zoom entries will not be added to the HUD text file.")

	# save the new image
	save_img_alphatest(canvas, "hud_weapon.bmp")

	print("Created HUD file: hud_output.bmp")

	with open("hud_output.txt", "w") as f:
		spr_count = 8 + (4 * has_ammo) + (4 * has_ammo2) + (4 * has_autoaim) + (4 * has_crosshair) + (4 * has_zoom)
		
		f.write("%d\n" % spr_count)
		if (has_ammo):		f.write("ammo			2560	hud_output	340 270 72	72\n")
		if (has_ammo2):		f.write("ammo2			2560	hud_output	412 270 72	72\n")
		if (has_autoaim):	f.write("autoaim			2560	hud_output	340 414 72	72\n")
		if (has_crosshair): f.write("crosshair		2560	hud_output	340 342 72	72\n")
		if (has_zoom):		f.write("zoom			2560	scope		0	0	%d	%d\n" % (zoom_size, zoom_size))
		f.write("weapon			2560	hud_output	0	0	510 135\n")
		f.write("weapon_s		2560	hud_output	0	135 510 135\n")
		if (has_ammo):		f.write("ammo			1280	hud_output	340 414 48	48\n")
		if (has_ammo2):		f.write("ammo2			1280	hud_output	388 414 48	48\n")
		if (has_autoaim):	f.write("autoaim			1280	hud_output	412 414 48	48\n")
		if (has_crosshair): f.write("crosshair		1280	hud_output	340 462 48	48\n")
		if (has_zoom):		f.write("zoom			1280	scope		0	0	%d	%d\n" % (zoom_size, zoom_size))
		f.write("weapon			1280	hud_output	0	270 340 90\n")
		f.write("weapon_s		1280	hud_output	0	360 340 90\n")
		if (has_ammo):		f.write("ammo			640		hud_output	484 270 24	24\n")
		if (has_ammo2):		f.write("ammo2			640		hud_output	484 294 24	24\n")
		if (has_autoaim):	f.write("autoaim			640		hud_output	460 414 24	24\n")
		if (has_crosshair): f.write("crosshair		640		hud_output	484 318 24	24\n")
		if (has_zoom):		f.write("zoom			640		scope		0	0	%d	%d\n" % (zoom_size, zoom_size))
		f.write("weapon			640		hud_output	0	450 170 45\n")
		f.write("weapon_s		640		hud_output	170 450 170 45\n")
		if (has_ammo):		f.write("ammo			320		hud_output	484 366 18	18\n")
		if (has_ammo2):		f.write("ammo2			320		hud_output	484 384 18	18\n")
		if (has_autoaim):	f.write("autoaim			320		hud_output	484 402 18	18\n")
		if (has_crosshair): f.write("crosshair		320		hud_output	484 318 24	24\n")
		if (has_zoom):		f.write("zoom			320		scope		0	0	%d	%d\n" % (zoom_size, zoom_size))
		f.write("weapon			320		hud_output	388 462 80	20\n")
		f.write("weapon_s		320		hud_output	388 482 80	20\n")
		
		print("Created HUD file: hud_output.txt")


def paste_hud_def(canvas, img, icon, res, spr, x, y, w, h):
	if w > img.width:
		img = img.resize((w, h), Image.NEAREST)
	elif w < img.width:
		img = img.convert("RGB").resize((w, h), Image.Resampling.LANCZOS)
	
	canvas.paste(img, (x, y))

def paste_hud_defs(canvas, img, hud_defs):
	bucket = Image.open(img + ".bmp")
	
	hud_config = []
	
	for icon in hud_defs:
		paste_hud_def(canvas, bucket, *icon)
		hud_config.append(icon)
		
	return hud_config


def compile_bucket_hud(out_name):
	hud_config = []
	bucket_icons = [
		'bucket1', 'bucket2', 'bucket3', 'bucket4', 'bucket5',
		'bucket6', 'bucket7', 'bucket8', 'bucket9', 'bucket10',
	]
	canvas = Image.new("RGB", (300, 200), "blue")
	w = 20
	h = 20
	
	for idx, icon in enumerate(bucket_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		x = (idx % 5)
		y = int(idx / 5)
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, x*w*3,		y*h*3,		 w*3, h*3),
			(icon, 1280, out_name, x*w*2,		y*h*2 + h*6, w*2, h*2),
			(icon, 640,	 out_name, x*w	+ w*10, y*h	  + h*6, w,	  h),
			(icon, 320,	 out_name, x*12 + w*10, y*12  + h*8, 12,  12)
		])
	
	save_img_alphatest(canvas, out_name)
	
	return hud_config

def compile_selection_hud(out_name):
	hud_config = []
	canvas = Image.new("RGB", (512, 225), "blue")
	w = 20
	h = 20
	
	if os.path.exists("selection.bmp"):
		icon = "selection"
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, 0, 0, 170*3, 45*3),
			(icon, 1280, out_name, 0, 45*3, 170*2, 45*2),
			(icon, 640,	 out_name, 170*2, 45*3, 170*1, 45*1),
			(icon, 320,	 out_name, 170*2, 45*3 + 45, 80, 20),
		])
	
	save_img_alphatest(canvas, out_name)
	
	return hud_config

def compile_number_hud(out_name):
	hud_config = []
	bucket_icons = [
		'number_0', 'number_1', 'number_2', 'number_3', 'number_4',
		'number_5', 'number_6', 'number_7', 'number_8', 'number_9',
	]
	canvas = Image.new("RGB", (300, 240), "blue")
	w = 20
	h = 24
	
	for idx, icon in enumerate(bucket_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		x = (idx % 5)
		y = int(idx / 5)
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, x*w*3,		y*h*3,		 w*3, h*3),
			(icon, 1280, out_name, x*w*2,		y*h*2 + h*6, w*2, h*2),
			(icon, 640,	 out_name, x*w	+ w*10, y*h	  + h*6, w,	  h),
			(icon, 320,	 out_name, x*12 + w*10, y*16  + h*8, 12,  16)
		])
	
	save_img_alphatest(canvas, out_name)
	
	return hud_config

def compile_damage_hud(out_name):
	hud_config = []
	bucket_icons = [
		'dmg_bio', 'dmg_poison', 'dmg_chem', 'dmg_cold', 'dmg_drown',
		'dmg_heat', 'dmg_gas', 'dmg_rad', 'dmg_shock',
	]
	
	w = 64
	h = 64
	
	for idx, icon in enumerate(bucket_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		canvas = Image.new("RGB", (320, 192), "blue")
		
		x = (idx % 5)
		y = int(idx / 5)
		
		out_name_part = out_name + "_" + icon
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name_part, 0, 0, w*3, h*3),
			(icon, 1280, out_name_part, w*3, 0, w*2, h*2),
			(icon, 640,	 out_name_part, w*3, h*2, w, h),
			(icon, 320,	 out_name_part, w*4, h*2, 32, 32)
		])
	
		save_img_alphatest(canvas, out_name_part)
	
	return hud_config

def compile_kill_hud(out_name):
	hud_config = []
	wide_icons = [
		'd_crowbar', 'd_9mmAR', 'd_shotgun', 'd_bolt',
		'd_crossbow', 'd_rpg_rocket', 'd_hornet',
	]
	norm_icons = [
		'd_9mmhandgun', 'd_357', 'd_gauss', 'd_egon', 'd_grenade',
		'd_satchel', 'd_tripmine', 'd_snark', 'd_skull', 'd_tracktrain',
	]

	canvas = Image.new("RGB", (256, 240), "blue")
	w = 32
	h = 16
	
	out_name_part = "hud_kill"
	
	for idx, icon in enumerate(norm_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		x = int(idx / 5)
		y = int(idx % 5)
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name_part, x*w*3,		 y*h*3,		  w*3, h*3),
			(icon, 1280, out_name_part, x*w*2 + w*6, y*h*2, w*2, h*2),
			(icon, 640,	 out_name_part, x*w	  + w*6, y*h + h*10, w,	  h),
			(icon, 320,	 out_name_part, x*w	 + w*8, y*16 + h*10, w,	 h)
		])
	
	save_img_alphatest(canvas, out_name_part)

	out_name_part = "hud_kill_wide"
	canvas = Image.new("RGB", (480, 192), "blue")
	w = 48
	
	for idx, icon in enumerate(wide_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		colh = 4
		x = int(idx / colh)
		y = int(idx % colh)
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name_part, x*w*3,		 y*h*3,		  w*3, h*3),
			(icon, 1280, out_name_part, x*w*2 + w*6, y*h*2, w*2, h*2),
			(icon, 640,	 out_name_part, x*w	  + w*6, y*h + h*colh*2, w,	  h),
			(icon, 320,	 out_name_part, x*w	 + w*8,  y*16 + h*colh*2, w,	 h)
		])
	
	save_img_alphatest(canvas, out_name_part)

	return hud_config

def compile_item_hud(out_name):
	hud_config = []
	bucket_icons = [
		'item_battery', 'item_healthkit', 'item_longjump',
	]
	
	canvas = Image.new("RGB", (396, 220), "blue")
	w = 44
	h = 44

	for idx, icon in enumerate(bucket_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		x = idx
		y = 0
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, x*w*3,		y*h*3,		 w*3, h*3),
			(icon, 1280, out_name, x*w*2,		y*h*2 + h*3, w*2, h*2),
			(icon, 640,	 out_name, x*w	+ w*3*2, y*h   + h*3, w,   h),
			(icon, 320,	 out_name, x*20 + w*3*2, y*16  + h*4, 20,  20)
		])
	
		save_img_alphatest(canvas, out_name)
	
	return hud_config

def compile_train_hud(out_name):
	hud_config = []
	bucket_icons = [
		'train_back', 'train_stop', 'train_forward1', 'train_forward2', 'train_forward3',
	]
	
	canvas = Image.new("RGB", (480, 384), "blue")
	w = 48
	h = 48

	for idx, icon in enumerate(bucket_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		colh = 2
		x = int(idx / colh)
		y = int(idx % colh)
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, x*w*3,		y*h*3,		 w*3, h*3),
		])
		
	for idx, icon in enumerate(bucket_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		colh = 1
		x = int(idx / colh)
		y = int(idx % colh)
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 1280, out_name, x*w*2,	48*6 + y*h*2, w*2, h*2),
		])
		
	for idx, icon in enumerate(bucket_icons):
		if not os.path.exists(icon + ".bmp"):
			continue
		
		colh = 2
		x = int(idx / colh)
		y = int(idx % colh)
		
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 640,	 out_name, x*w + w*3*2, y*h + h*3, w,	 h),
			(icon, 320,	 out_name, w*3*3, idx*h, w,	 h),
		])
	
		save_img_alphatest(canvas, out_name)
	
	return hud_config

def compile_suit_hud(out_name):
	hud_config = []
	
	canvas = Image.new("RGB", (400, 280), "blue")
	w = 40
	h = 40

	if os.path.exists("cross.bmp"):
		icon = "cross"
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, 0, 0, 32*3, 32*3),
			(icon, 1280, out_name, 0, 32*3, 32*2, 32*2),
			(icon, 640,	 out_name, 32*6, 32*3, 32*1, 32*1),
			(icon, 320,	 out_name, 32*6, 32*4, 16, 16),
		])
	if os.path.exists("flash_full.bmp"):
		icon = "flash_full"
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, 32*3, 0, 32*3, 32*3),
			(icon, 1280, out_name, 32*2, 32*3, 32*2, 32*2),
			(icon, 640,	 out_name, 32*7, 32*3, 32*1, 32*1),
			(icon, 320,	 out_name, 32*6 + 16, 32*4, 16, 16),
		])
	if os.path.exists("flash_empty.bmp"):
		icon = "flash_empty"
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, 32*6, 0, 32*3, 32*3),
			(icon, 1280, out_name, 32*4, 32*3, 32*2, 32*2),
			(icon, 640,	 out_name, 32*8, 32*3, 32*1, 32*1),
			(icon, 320,	 out_name, 32*6 + 16*2, 32*4, 16, 16),
		])
	if os.path.exists("flash_beam.bmp"):
		icon = "flash_beam"
		x = 32*3*3
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, x, 0, 16*3, 32*3),
			(icon, 1280, out_name, x + 16*3, 0, 16*2, 32*2),
			(icon, 640,	 out_name, x + 16*5, 0, 16*1, 32*1),
			(icon, 320,	 out_name, x + 16*6, 0, 6, 16),
		])
	if os.path.exists("suit_full.bmp"):
		icon = "suit_full"
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, 0, 32*5, w*3, h*3),
			(icon, 1280, out_name, w*6, 32*5, w*2, h*2),
			(icon, 640,	 out_name, w*6, 32*5 + 40*2 , w*1, h*1),
			(icon, 320,	 out_name, w*8, 32*5 + 40*2, 20, 20),
		])
	if os.path.exists("suit_empty.bmp"):
		icon = "suit_empty"
		hud_config += paste_hud_defs(canvas, icon, [
			(icon, 2560, out_name, w*3, 32*5, w*3, h*3),
			(icon, 1280, out_name, w*8, 32*5, w*2, h*2),
			(icon, 640,	 out_name, w*7, 32*5 + 40*2 , w*1, h*1),
			(icon, 320,	 out_name, w*8 + 20, 32*5 + 40*2, 20, 20),
		])
	
	save_img_alphatest(canvas, out_name)
	
	return hud_config

def tab_align(tup, idx, maxtabs):
	tabsize = 4
	left = str(tup[idx])
	used = len(left) // tabsize + 1
	tabs = "\t" * (maxtabs - used if used < maxtabs else 1)

	lst = list(tup)
	lst[idx] = str(lst[idx]) + tabs
	return tuple(lst)

def compile_system_hud():
	bucket_img = Image.new("RGB", (512, 512), "blue")

	hud_config = [
		compile_selection_hud("hud_selection"),
		compile_bucket_hud("hud_buckets"),
		compile_number_hud("hud_number"),
		compile_damage_hud("hud"),
		compile_kill_hud("hud"),
		compile_item_hud("hud_item"),
		compile_train_hud("hud_train"),
		compile_suit_hud("hud_suit"),
	]

	total_spr = 0
	for icon_set in hud_config:
		total_spr += len(icon_set)

	with open("hud.txt", "w") as f:
		f.write("%d\n" % total_spr)
		
		for icon_set in hud_config:
			for icon in icon_set:
				icon = tab_align(icon, 0, 5)
				icon = tab_align(icon, 1, 3)
				
				f.write("%s%s%s %d	%d	%d	%d\n" % (icon[0], icon[1], "mod_folder/" + icon[2], icon[3], icon[4], icon[5], icon[6]))
			f.write("\n")
		print("Created HUD file: hud_system.txt")


compile_mode = len(sys.argv) > 1 and sys.argv[1]

if len(sys.argv) < 1 or (not compile_mode):
	print("This script creates HL25 compatible HUD sprites and configs for all resolutions.\n")
	print("Compiling a weapon hud:")
	print(" hud_create.py weapon [weapon_hud.txt]")
	print(" If weapon_hud.txt is added. Bitmaps will be extracted from there first.\n")
	print(" Compiling uses the following files:")
	print(" weapon.bmp = deselected weapon icon (170x45)")
	print(" weapon_s.bmp = selected weapon icon (170x45)\n")

	print(" Optional icons:")
	print(" ammo.bmp = primary ammo icon (24x24)")
	print(" ammo2.bmp = secondary ammo icon (24x24)")
	print(" crosshair.bmp = weapon crosshair (24x24)")
	print(" autoaim.bmp = autoaim crosshair (24x24)")
	print(" zoom.bmp = scope sprite (size doesn't matter)")
	
	print("\nCompiling a system hud:")
	print(" hud_create.py system [hud.txt]\n")
	print(" If hud.txt is added. Bitmaps will be extracted from there first.")
	print(" Compiling uses the following files, all of which are optional:")
	print(" number_[0-9].bmp = uhhh (24x24)")
	sys.exit(1)

if len(sys.argv) > 2 and sys.argv[2]:
	hud_path = sys.argv[2]
	process_definition_file(hud_path)

if compile_mode == "weapon":
	compile_weapon_hud()
if compile_mode == "system":
	compile_system_hud()