# SevenKewp
The goal for this project is to become a generic Half-Life co-op mod which is easy to customize. Default content and gameplay will stay faithful to classic Half-Life. That means there won't be any: tacticool weapons, HD models, bullet-sponge enemies, etc. This isn't strictly a Sven Co-op clone, but recreating its entities and features is a high priority because nearly all GoldSrc co-op maps were made for sven.

View project status [here](https://github.com/wootguy/SevenKewp/issues/8).

This repo was forked from an early version of [halflife-updated](https://github.com/twhl-community/halflife-updated/tree/8cb9d9eb9016ff56fcba099a09a3b6e6563853b1) (Nov 2021).

# Building the mod
Note: If you're a player, you don't need to install anything or build this mod. Just join a server showing "Half-Life Co-op" in the game column. The mod is totally server-side. A custom client might be created later but it will be optional.  

1. Install dependendies.  
   Windows: Install Git, CMake, and Visual Studio  
   Debian: `apt install git cmake build-essential gcc-multilib g++-multilib libc6-dev-i386`  
1. Run `scripts/build_game_and_plugins.bat` (Windows) or `scripts/build_game_and_plugins.sh` (Linux).
2. Copy the contents of `build/output/` to `valve/` on your Half-Life dedicated server.
3. Build and install [my fork of ReHLDS](https://github.com/wootguy/rehlds). This is required for some maps.
1. Copy the contents `sevenkewp` to `valve_downloads/` on your dedicated server. You may want/need the `.cfg` files in `valve/` instead.
1. Add `-dll dlls/server.dll` (Windows) or `-dll dlls/server.so` (Linux) to the launch options of your dedicated server. If you want metamod, then add that path to metamod's `config.ini` instead.

Currently, the mod is designed to run as a replacement server library for Half-Life, rather than a new mod with its own server list and client. The mod doesn't work with Sven Co-op clients yet.

# Building native plugins
Native plugins are tightly integrated with the mod code and have full access to its classes and utilities. This makes native plugins powerful but easily broken by mod updates. Here's how to build them.

1. Set up the mod project as described in the previous section
2. Open a shell in `SevenKewp/plugins/`
3. Clone all the plugin repos you want. Example: `git clone https://github.com/wootguy/SevenKewp_plugins/`
4. Run the build script again: `scripts/build_game_and_plugins.bat` (Windows) or `scripts/build_game_and_plugins.sh` (Linux)
5. Copy the contents of `SevenKewp/build/output/` to `valve/` on your dedicated server.
6. Repeat steps 4 and 5 whenever the mod updates.

# Installing native plugins
Once you have built or downloaded a native plugin, you need to copy it to the right place and update the relevant config file. `valve` and `valve_downloads` are interchangeable. The plugin loader will search both paths.

## Server plugins
1. Copy the dll to the server plugins folder (Example: `valve/plugins/server/AntiBlock.dll`)
2. Add the plugin file path to `valve/plugins.txt` without the file extension (Example: write `Antiblock` to load the path in step 1)

Plugin paths in `plugins.txt` start in `valve/plugins/server/`. That's why you only need to write `AntiBlock` instead of `valve/plugins/server/AntiBlock` in the above example.

## Map plugins
These steps only apply to unreleased maps/ports. Ideally the map package will set this up for you so that you only need to extract the files.

1. Copy the dll to the map plugins folder (Example: `valve/plugins/maps/pizza_ya_san.dll`)
2. Add the plugin file path to the map cfg file(s) the plugin was made for (Example: add `map_plugin pizza_ya_san` to `valve/maps/pizza_ya_san1.cfg` and `valve/maps/pizza_ya_san2.cfg`).

Plugin paths in map cfg files start in `valve/plugins/maps/`.

# Installing Sven Co-op maps
Sven Co-op maps often need converting before they can be used in Half-Life. Not doing this will result in crashes. This section is here mostly as a warning not to install maps directly. Good luck actually following this. I plan to publish all my ports and ripents later so that this isn't needed.

1. Create a folder and copy `scripts/convert_map.py` to the new folder.
2. Add modelguy, ripent, bspguy, and wadmaker to the same folder. modelguy and bspguy need to be built from source because the latest releases are missing features.
3. Copy `resguy_default_content_hl.txt` [from resguy](https://github.com/wootguy/resguy/blob/master/config/resguy_default_content_hl.txt) to the folder, and rename it to `resguy_default_content.txt`
4. Update `valve_path` in the script to point to your `Half-Life/valve` folder.
5. Install imagemagick and ffmpeg. Make sure `magick` and `ffmpeg` are usable from the command-line.
1. Extract a map package to the folder (`maps/` should end up next to the script)
2. Run the script

The script will fail on certain things like models with invalid textures. Those models need manual edits. Maps made for Sven Co-op 5.x sometimes need special attention (map moving/splitting to fit in the +/-4096 grid, texture resizing, etc.). The master branch of [bspguy](https://github.com/wootguy/bspguy) has porting tools to help with this.

So far, I've had the only server hosting sven maps so I don't bother renaming files that the script modifies. That's a horrible practice and will cause crashes once other server ops start hosting without reading this section.
