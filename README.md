# Chip8++

Simple, cross-platform, SUPERCHIP emulator written in C++23 using SDL3.

<details>
    <summary>
        File Structure
    </summary>

```text
.
├-- CMakeLists.txt
|   CMake project and build configuration
|
├-- CMakePresets.json
|   Presets for setting up CMake
|
├-- justfile
|   Common commands for setup, building, and running the project
|
├-- .clang-format
|   Specifications for how to format the code
|
├-- src
|   ├-- main.cpp
|   |   Initialisation, ROM loading, and the main run loop
|   |
|   ├-- core.hpp
|   ├-- core.cpp
|   |   Emulator core with the machine state class and ticking methods
|   |
|   └-- default_font.hpp
|       Default small and big fonts
|
├-- deps/SDL
|   Git submodule with the SDL3 sources
|
├-- .vscode
|   ├-- tasks.json
|   |   Build tasks for VS Code
|   |
|   └-- launch.json
|       Debug launch profile for VS Code
|
└-- build
    Scratch directory for building
```

</details>

## Building

This project uses CMake, you will need to install CMake, as well as a compatible C++ toolchain such as LLVM Clang, GNU GCC, or MSVC.

You can use [Just](https://just.systems) to make setup and running from the command line easier, or use VS Code with the included build task and debug launch configuration.

1. Cloning
    ```
    git clone --recursive https://github.com/theRookieCoder/Chip8PP
    ```
    Make sure you are cloning the submodules too, since SDL3 is built from source.

2. Setup CMake
    ```
    just cmake [preset]
    ```
    Use the `cmake` recipe to generate the build files. The `Debug` or `Debug (Windows)` preset is selected by default depending on your OS. You can also specify a user preset (defined in `CMakeUserPresets.json`), or one of the following presets
    - `Debug` and `Release`  
        For use with UNIX platforms such as macOS and Linux
    - `Debug (Windows)` and `Release (Windows)`  
        For use with Windows and Microsoft Visual C++

3. Build an executable
    ```
    just
    # or
    just build
    ```
    Build an executable, which is output to `build/Chip8PP` or `build\Chip8PP.exe`. On Windows, the DLLs are automatically moved to the same directory as the executable.

4. Run the executable
    ```
    just run rom_file
    ```
    Build and run the executable with the specified `rom_file`.

## Contributing

Pull requests are welcome. Make sure you format your code, use consistent naming, and test your changes. If you are planning to make large changes, contact me on Discord first (username `therookiecoder`).
