cmake_minimum_required(VERSION 3.29)

project(Chip8PP)
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(deps/SDL REQUIRED)

file(GLOB src_files CONFIGURE_DEPENDS "src/*.hpp" "src/*.cpp")

add_executable(${PROJECT_NAME} ${src_files})
target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:${PROJECT_NAME}> $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
    COMMAND_EXPAND_LISTS
)
