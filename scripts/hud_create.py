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

compile_mode = sys.argv[1] == "compile"

if len(sys.argv) < 1 or (not compile_mode):
	print("Usage:")
	print("hud_create.py compile [sven_weapon_hud.txt]\n")
	print("Compiling uses the following files:")
	print("weapon.bmp = deselected weapon icon (170x45)")
	print("weapon_s.bmp = selected weapon icon (170x45)\n")

	print("Optional icons:")
	print("ammo.bmp = ammo icon (24x24)")
	print("crosshair.bmp = weapon crosshair (24x24)")
	print("autoaim.bmp = autoaim crosshair (24x24)")
	print("zoom.bmp = scope sprite (size doesn't matter)")
	sys.exit(1)

if sys.argv[2]:
	hud_path = sys.argv[2]
	process_definition_file(hud_path)


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

has_ammo = False
has_crosshair = False
has_autoaim = False
has_zoom = False
zoom_size = 512

# ammo icons
if os.path.exists("ammo.bmp"):
	ammo = Image.open('ammo.bmp')
	new_img.paste(ammo.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (340, 270))
	new_img.paste(ammo.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (412, 270))
	new_img.paste(ammo, (460, 270))
	has_ammo = True
else:
	print("ammo.bmp does not exist. Ammo icons will not be added to the HUD.")

# crosshair icons
if os.path.exists("crosshair.bmp"):
	crosshair = Image.open('crosshair.bmp')
	new_img.paste(crosshair.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (340, 342))
	new_img.paste(crosshair.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (412, 342))
	new_img.paste(crosshair, (460, 342))
	has_crosshair = True
else:
	print("crosshair.bmp does not exist. Crosshairs will not be added to the HUD.")

# autoaim icons
if os.path.exists("autoaim.bmp"):
	autoaim = Image.open('autoaim.bmp')
	new_img.paste(autoaim.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (340, 414))
	new_img.paste(autoaim.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (412, 414))
	new_img.paste(autoaim, (460, 414))
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
new_img.quantize(colors=256, method=2).save("hud_output.bmp")

print("Created HUD file: hud_output.bmp")

with open("hud_output.txt", "w") as f:
	spr_count = 6 + (3 if has_ammo else 0) + (3 if has_autoaim else 0) + (3 if has_crosshair else 0) + (3 if has_zoom else 0)
	
	f.write("%d\n" % spr_count)
	if (has_ammo):		f.write("ammo			2560	hud_output	340	270	72	72\n")
	if (has_autoaim):	f.write("autoaim			2560	hud_output	340	414	72	72\n")
	if (has_crosshair): f.write("crosshair		2560	hud_output	340	342	72	72\n")
	if (has_zoom):		f.write("zoom			2560	scope		0	0	%d	%d\n" % (zoom_size, zoom_size))
	f.write("weapon			2560	hud_output	0	0	510	135\n")
	f.write("weapon_s		2560	hud_output	0	135	510	135\n")
	if (has_ammo):		f.write("ammo			1280	hud_output	412	270	48	48\n")
	if (has_autoaim):	f.write("autoaim			1280	hud_output	412	414	48	48\n")
	if (has_crosshair):	f.write("crosshair		1280	hud_output	412	342	48	48\n")
	if (has_zoom):		f.write("zoom			1280	scope		0	0	%d	%d\n" % (zoom_size, zoom_size))
	f.write("weapon			1280	hud_output	0	270	340	90\n")
	f.write("weapon_s		1280	hud_output	0	360	340	90\n")
	if (has_ammo):		f.write("ammo			640		hud_output	460	270	24	24\n")
	if (has_autoaim):	f.write("autoaim			640		hud_output	460	414	24	24\n")
	if (has_crosshair):	f.write("crosshair		640		hud_output	460	342	24	24\n")
	if (has_zoom):		f.write("zoom			640		scope		0	0	%d	%d\n" % (zoom_size, zoom_size))
	f.write("weapon			640		hud_output	0	450	170	45\n")
	f.write("weapon_s		640		hud_output	170	450	170	45\n")
	
	print("Created HUD file: hud_output.txt")

