set windows-shell := ["powershell"]

default: build

cmake preset="debug":
    cmake --preset {{ preset }}

[working-directory: "build"]
build:
    make -j$(nproc)

run rom: build
    ./build/Chip8PP "{{ rom }}"

clean:
    rm -rf build
