cmake_minimum_required(VERSION 3.5)
project(Chip8PP)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(deps/SDL EXCLUDE_FROM_ALL)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
