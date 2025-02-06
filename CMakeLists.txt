cmake_minimum_required(VERSION 3.30)

project(Chip8PP)
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(deps/SDL EXCLUDE_FROM_ALL)

file(GLOB src_files CONFIGURE_DEPENDS "src/*.hpp" "src/*.cpp")

add_executable(${PROJECT_NAME} ${src_files})
target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
