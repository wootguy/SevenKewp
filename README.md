# SevenKewp
The goal for this project is to become a generic Half-Life co-op mod which is easy to customize. Default content and gameplay will stay faithful to classic Half-Life. That means there won't be any: tacticool weapons, HD models, bullet-sponge enemies, etc. This isn't strictly a Sven Co-op clone, but recreating its entities and features is a high priority because nearly all GoldSrc co-op maps were made for sven.

View project status [here](https://github.com/wootguy/SevenKewp/issues/8).

This repo was forked from an early version of [halflife-updated](https://github.com/twhl-community/halflife-updated/tree/8cb9d9eb9016ff56fcba099a09a3b6e6563853b1) (Nov 2021).

# Building the mod
### Windows users:
1. Install Git, CMake, and Visual Studio
1. Open a command prompt somewhere and run these commands:
    ```
    git clone --recurse-submodules https://github.com/wootguy/SevenKewp.git
    cd SevenKewp && mkdir build && cd build
    cmake .. -A Win32
    cmake --build . --config Release
    ```
1. Copy the `sevenkewp` folder to your `Half-Life` or `Sven Co-op` folder.
1. Add `-game sevenkewp` to the launch options of Half-Life or Sven Co-op. Launch the game and then type `map sc_test` in console to test out the mod.

TODO: New instructions needed. Currently, the mod is designed to run as a replacement server dll for Half-Life, rather than a standalone `-game`. Many SC maps also need ripenting for things I refuse to add support for in code (legacy entity logic and audio formats, mostly).
