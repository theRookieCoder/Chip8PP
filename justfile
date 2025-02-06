set windows-shell := ["powershell"]

default: build

# Generate build files using CMake with the specified preset ("debug" by default)
cmake preset="debug":
    cmake --preset {{ preset }}

# Build an executable in the build/ directory
build:
    cmake --build build --parallel $(nproc)

# Build and run the executable with the specified ROM file
run rom: build
    ./build/Chip8PP "{{ rom }}"

# Delete the build/ directory
clean:
    rm -rf build
