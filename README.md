# Chip8++

Simple, cross-platform, SUPERCHIP emulator written in C++23 using SDL3.

<details>
    <summary>
        File Structure
    </summary>

```text
.
├── CMakeLists.txt
│   CMake project and build configuration
│
├── CMakePresets.json
│   Presets for setting up CMake
│
├── justfile
│   Common commands for setup, building, and running the project
│
├── .clang-format
│   Specifications for how to format the code
│
├── src
│   ├── main.cpp
│   │   Initialisation, ROM loading, and the main run loop
│   │
│   ├── core.hpp
│   ├── core.cpp
│   │   Emulator core with the machine state class and ticking methods
│   │
│   └── default_font.hpp
│       Default small and big fonts
│
├── deps/SDL
│   Git submodule with the SDL3 sources
│
├── .vscode
│   ├── tasks.json
│   │   Build tasks for VS Code
│   │
│   └── launch.json
│       Debug launch profile for VS Code
│
└── build
    Scratch directory for building
```

</details>

## Building

This project uses CMake, you will need to install it as well as a compatible C++ toolchain such as LLVM Clang, GNU GCC, or MSVC.

You can optionally use [Just](https://just.systems) to make setup and running from the command line easier, or use VS Code with the included build task and debug launch configuration.
If you are using CLion, make sure you enable the included CMake presets if you would like to use them.

### Cloning

```shell
git clone --recursive https://github.com/theRookieCoder/Chip8PP
```

_Make sure you are cloning the submodules too_, since SDL3 is built from source.

### CMake Presets

You can use a user preset (defined in `CMakeUserPresets.json`) or one of the following included presets
- `debug` and `release`  
  For use with UNIX platforms such as macOS and Linux
- `debug-windows` and `release-windows`  
  For use with Windows and Microsoft Visual C++

### Setup CMake

```shell
cmake --preset preset-name
# OR
just cmake-gen [preset-name]
```

You can use the `cmake-gen` recipe to generate the build files. The `Debug` or `Debug (Windows)` preset will be selected by default depending on your OS.

### Build an Executable

```shell
# Debug:
cmake --build build-debug --parallel
# OR
just # aliased to `just build`

# Release:
cmake --build build-release --parallel
# OR
just build-rel
```

The executable is output to `build-debug/Chip8PP` or `build-release/Chip8PP`. On Windows, the DLLs are automatically moved to the same directory as the executable.

## Contributing

Pull requests are welcome. Make sure you format your code, use consistent naming, and test your changes. If you are planning to make large changes, contact me on Discord first (username `therookiecoder`).
