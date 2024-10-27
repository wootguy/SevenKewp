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

# Building plugins
This mod has its own plugin system, similar to metamod. Here's how you build those plugins.

1. Set up the mod project as described in the previous section
2. Open a shell in `SevenKewp/plugins/`
3. Clone all the plugin repos you want. Example: `git clone https://github.com/wootguy/SevenKewp_plugins/`
4. Run the build script again: `scripts/build_game_and_plugins.bat` (Windows) or `scripts/build_game_and_plugins.sh` (Linux)
5. Copy the contents of `SevenKewp/build/output/` to `valve/` on your dedicated server.
6. Repeat steps 4 and 5 whenever the mod updates.

Mod plugins are tightly coupled to the mod code and have full access to its classes and utilities. This makes plugins powerful but easily broken by mod updates.
