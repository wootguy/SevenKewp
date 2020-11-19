# SevenKewp

This will probably be abandoned in a few months and never touched again. I just want to learn how to make a mod, mostly.

Forked from: [halflife-updated](https://github.com/Solokiller/halflife-updated)

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
