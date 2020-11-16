# Building the source
### Windows users:
1. Install CMake and Visual Studio
1. Download and extract the source somewhere
1. Open a command prompt in the `halflife-updated` folder and run these commands:
    ```
    mkdir build && cd build
    cmake .. -A Win32
    cmake --build . --config Release
    ```