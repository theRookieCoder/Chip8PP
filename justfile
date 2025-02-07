set windows-shell := ["powershell"]

default: build

default_preset := if os_family() == "windows" { "debug-windows" } else { "debug" }

# Generate build files using CMake with the specified preset
cmake preset=default_preset:
    cmake --preset {{ preset }}

# Build an executable in the build/ directory
build:
    cmake --build build --parallel

# Build and run the executable with the specified ROM file
run rom: build
    ./build/Chip8PP "{{ rom }}"

# Delete the build/ directory
[unix]
clean:
    rm -rf build

# Delete the build/ directory
[windows]
clean:
    if (Test-Path build) { Remove-Item -Recurse build }
