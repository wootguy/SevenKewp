from PIL import Image
import os, sys

# creates a weapon HUD file for HL25 for all resolutions, packed into a single 512x512 image

# Prepare the following input icons:
# weapon.bmp = deselected weapon icon (170x45)
# weapon_s.bmp = selected weapon icon (170x45)

# Optional icons:
# ammo.bmp = ammo icon (24x24)
# crosshair.bmp = weapon crosshair (24x24)
# autoaim.bmp = autoaim crosshair (24x24)

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

# ammo icons
if os.path.exists("ammo.bmp"):
	ammo = Image.open('ammo.bmp')
	new_img.paste(ammo.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (340, 270))
	new_img.paste(ammo.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (412, 270))
	new_img.paste(ammo, (460, 270))
else:
	print("ammo.bmp does not exist. Ammo icons will not be added to the HUD.")

# crosshair icons
if os.path.exists("crosshair.bmp"):
	crosshair = Image.open('crosshair.bmp')
	new_img.paste(crosshair.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (340, 342))
	new_img.paste(crosshair.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (412, 342))
	new_img.paste(crosshair, (460, 342))
else:
	print("crosshair.bmp does not exist. Crosshairs will not be added to the HUD.")

# autoaim icons
if os.path.exists("autoaim.bmp"):
	autoaim = Image.open('autoaim.bmp')
	new_img.paste(autoaim.resize((int(24 * 3), int(24 * 3)), Image.NEAREST), (340, 414))
	new_img.paste(autoaim.resize((int(24 * 2), int(24 * 2)), Image.NEAREST), (412, 414))
	new_img.paste(autoaim, (460, 414))
else:
	print("autoaim.bmp does not exist. Autoaim crosshairs will not be added to the HUD.")

# save the new image
new_img.quantize(colors=256, method=2).save("hud_output.bmp")

print("Created HUD file: hud_output.bmp")
