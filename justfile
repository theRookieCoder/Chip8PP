set windows-shell := ["powershell"]

default: build

default_preset := if os_family() == "windows" { "debug-windows" } else { "debug" }

# Generate build files using CMake with the specified preset
cmake-gen preset=default_preset:
    cmake --preset {{ preset }}

# Build a debug executable in the build-debug/ directory
build dir="build-debug":
    cmake --build {{ dir }} --parallel

# Build a release executable in the build-release/ directory
build-rel: (build "build-release")
