# SevenKewp
The goal for this project is to become a generic Half-Life co-op mod which is easy to customize. Default content and gameplay will stay faithful to classic Half-Life. That means there won't be any: tacticool weapons, HD models, bullet-sponge enemies, etc. This isn't strictly a Sven Co-op clone, but recreating its entities and features is a high priority because nearly all GoldSrc co-op maps were made for sven. The mod is designed to run as a Half-Life addon, rather than a new mod with its own server list and launcher.

View project status [here](https://github.com/wootguy/SevenKewp/issues/8).

This repo was forked from an early version of [halflife-updated](https://github.com/twhl-community/halflife-updated/tree/8cb9d9eb9016ff56fcba099a09a3b6e6563853b1) (Nov 2021).

# Building the client
If you're a player, you don't need to install anything or build this mod. Just join a server showing "Half-Life Co-op" in the game column. Vanilla Half-Life clients are able to play with a few limitations.

### 2025/08/17: The client isn't ready yet. It will not work properly if you join the server using it. I updated this page early not thinking anyone would try bulding it. I need about a week or 2 to get a testable version ready. I'm planning to add an auto-updater so you don't need to manually install a new version every day.

Build instructions:
1. Install dependendies.  
   Windows: Install Git, CMake, and Visual Studio  
   Debian: `apt install git cmake build-essential gcc-multilib g++-multilib libc6-dev-i386 mesa-common-dev libssl-dev:i386`
1. Open a shell somewhere and clone the repo: `git clone --recurse-submodules https://github.com/wootguy/SevenKewp`
1. Run `scripts/build_client.bat` (Windows) or `scripts/build_client.sh` (Linux).
1. Copy the contents of `SevenKewp/build_client/output/` to `Steam/steamapps/common/Half-Life/valve_addon/`.
   - Create the `valve_addon` folder if it doesn't already exist.
1. Launch the game and enable `Options` -> `Content` -> `Allow custom addon content`
   - This forces the game to load files in `valve_addon` before `valve`, meaning the client library you just built will be loaded in place of the default one.
1. Try it out! Join a Half-Life Co-op server. Some features are disabled if you join a deathmatch server to prevent cheating (e.g. thirdperson camera)

### WARNING: USE CUSTOM CLIENTS AT YOUR OWN RISK

Executables distributed outside of Steam are subject to Valve Anti-Cheat (VAC) detection. Running this client may trigger VAC protections and result in a permanent VAC ban on your Steam account.

That said, I have observed players using the [HLBugfixed](https://github.com/tmp64/BugfixedHL-Rebased) and [Adrenaline Gamer](https://openag.pro/) clients for one year on my secure server without them being VAC banned. According to my understanding of the [VAC documentation](https://partner.steamgames.com/doc/features/anticheat), executables need to be manually reported as a cheat to Valve for its signature to be detected. I have not added cheats to this client.

# Building the server
Note: If you're a player, you don't need to install anything or build this mod. Just join a server showing "Half-Life Co-op" in the game column. The mod is totally server-side. A custom client might be created later but it will be optional.  

1. Install dependendies.  
   Windows: Install Git, CMake, and Visual Studio  
   Debian: `apt install git cmake build-essential gcc-multilib g++-multilib libc6-dev-i386 mesa-common-dev libssl-dev:i386`
1. Open a shell somewhere and clone the repo: `git clone --recurse-submodules https://github.com/wootguy/SevenKewp`
1. Run `scripts/build_game_and_plugins.bat` (Windows) or `scripts/build_game_and_plugins.sh` (Linux).
2. Intall the official Half-Life dedicated server from [SteamCMD](https://developer.valvesoftware.com/wiki/SteamCMD#Downloading_SteamCMD). The `steam_legacy` beta is needed for ReHLDS compatibility:
   ```
   login anonymous
   app_set_config 90 mod cstrike
   app_update 90 -beta steam_legacy validate
   ```
3. Copy the contents of `SevenKewp/build/output/` to `Half-Life/valve/` in your Half-Life dedicated server.
4. Build and install [my fork of ReHLDS](https://github.com/wootguy/rehlds). This is required for some maps.
    1. Windows users:
        1. Open `rehlds/msvc/ReHLDS.sln`. Select `OK` if asked to retarget.
        4. Optional: If you're setting up a public server, right-click the `ReHLDS` project and select `Properties` -> `Configuration Manager`. Change the `Active solution configuration` to `Release`. This gives much better performance.
        5. Right click `Solution 'ReHLDS'` and select `Build Solution`
        6. Copy the `.exe` and `.dll` files from `SevenKewp/rehlds/msvc/Debug` (or `Release`) over the existing files in your `Half-Life/` folder on the dedicated server. `Director.dll` is special and goes into `Half-Life/valve/dlls/` instead.
1. Copy the contents of `SevenKewp/sevenkewp/` to `Half-Life/valve/` on your dedicated server. The `.cfg` files must be in `valve/`, but you can copy everything else to `valve_downloads/` if you want.
1. Add `-dll dlls/server.dll` (Windows) or `-dll dlls/server.so` (Linux) to the launch options of your dedicated server. If you use metamod, then add `gamedll dlls/server.dll` (or `.so`) to metamod's `config.ini` instead.  
1. Try it out! Example launch command:  
   `hlds.exe +map c1a1 -dll dlls/server.dll +maxplayers 32 +developer 1 -heapsize 65536 -num_edicts 4096 +servercfg "" +log on`

# Building native plugins
Native plugins are tightly integrated with the mod code and have full access to its classes and utilities. This makes native plugins powerful but easily broken by mod updates. Here's how to build them.

1. Set up the mod project as described in the previous section
2. Open a shell in `SevenKewp/plugins/`
3. Clone all the plugin repos you want. Example: `git clone https://github.com/wootguy/HLCMapPlugins/`
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
1. Copy `skill.cfg` to the folder.
1. Copy `titles.txt` from the latest version of Sven Co-op to the folder.
4. Update `valve_path` in the script to point to your `Half-Life/valve` folder.
5. Install imagemagick and ffmpeg. Make sure `magick` and `ffmpeg` are usable from the command-line.
1. Extract a map package to the folder (`maps/` should end up next to the script)
2. Run the script

The script will fail on certain things like models with invalid textures. Those models need manual edits. Maps made for Sven Co-op 5.x sometimes need special attention (map moving/splitting to fit in the +/-4096 grid, texture resizing, etc.). The master branch of [bspguy](https://github.com/wootguy/bspguy) has porting tools to help with this.

So far, I've had the only server hosting sven maps so I don't bother renaming files that the script modifies. That's a horrible practice and will cause crashes once other server ops start hosting without reading this section.

# Setting up Visual Studio for development
This section will set you up to develop/debug the mod and native plugins.  

1. Set up the mod as described [here](https://github.com/wootguy/SevenKewp?tab=readme-ov-file#building-the-mod)
2. Open `SevenKewp/build/sevenkewp.sln`
3. Right click the `server` project and select `Properties`.  
4. Set `General` -> `Output Directory` to the `dlls` path on your dedicated server (`Half-Life/valve/dlls`)  
5. Set up the `Debugging` section:  
    - Set `Command` to `hlds.exe`  
    - Set `Command Arguments` to whatever you want to launch with.  
       Example: `+map sc_test -dll dlls/server.dll -num_edicts 2047 -heapsize 65536 +maxplayers 32 +developer 1 +sv_cheats 1 +log on`  
   - Set the `Working Directory` to your server path (`Half-Life/`). The path must end with a slash.
6. Repeat step 4-5 for any other configurations you want to use (Debug/Release/...)  
7. Right click the `server` project and select `Set as Startup Project`.  
8. Press F5. The server should start with the debugger attached.

<details>
<summary>Visual aids</summary>  
      
![image](https://github.com/user-attachments/assets/5651c1b6-f9ef-48ff-b1b6-754221f12a6a)  
![image](https://github.com/user-attachments/assets/b7078bac-b1a8-4487-8b28-bf02670522a9)
  
</details>

## Creating a new plugin

You can make a new plugin by duplicating an existing one and renaming the project inside `CMakeLists.txt`. A simple one to copy is `plugins/HLCoopMapPlugins/restriction/`. You may want to change the `hlcoop_setup_plugin()` line too depending on which type of plugin you're making. Map plugins use `"plugins/maps/"` and server plugins use `"plugins/server/"`. Map plugins function the same as server plugins except that they're loaded/unloaded by specific maps via .cfg files. Server plugins are always loaded.

### Adding the plugin to the solution
New plugins don't show up in Visual Studio until you reconfigure CMake. To do that:  

1. Open the CMake GUI  
2. Set the `source code` and `binaries` directories to `SevenKewp` and `SevenKewp/build` respectively.  
3. Click `Configure`, and then `Generate`  
4. If all goes well, Visual Studio should ask you to reload the projects and you'll see the new project. You may need to update your `CMakeLists.txt` file if there was an error.
  <details>
  <summary>Visual aid</summary>  
      
  ![image](https://github.com/user-attachments/assets/0d530473-3dee-4cb9-8c7e-d43567ed8782)

  </details>

## Troubleshooting
| Problem | Solution |
| --- | --- |
| `Unable to load engine, image is corrupt` &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; | Check that the server.dll file is in the right place. Be sure you installed the `steam_legacy` version of hlds from SteamCMD. |
| Crash on startup or when loading plugins | Right click the `Solution` and choose `Rebuild Solution`. This is necessary when changing build modes or updating APIs. Make sure the .dll files are in the right place, too. Each mode (Debug/Release/...) has a separate configuration you need to configure. |
| Dead on join and can't respawn | Make sure `sevenkewp/server.cfg` was copied to the `valve/` folder on your dedicated server. CFG files must be in `valve/` |

## Tips

- Don't use the `build_game_and_plugins` script while developing. It discards all local changes not pushed to github.
- Change the startup project to the one you're actively working on. This way, building with F5 will pick up changes and build only what's needed to test that project.
- ASAN is great for debugging memory corruption (crashes that make no sense or have corrupted stacks pointing to nowhere). Enable in the project setting `C/C++` -> `General` -> `Enable Address Sanitizer`. The server will run much slower so it's not something you want to have on normally.
- Visual Studio can't debug the server and client at the same time. Create a new build folder and solution via CMake if you want to do that.
