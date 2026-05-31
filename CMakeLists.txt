cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set project name.
project(hob_engine)

# Use vcpkg to find packages.
if (CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif ()

# Find all C++ source files recursively.
file(GLOB_RECURSE CPP_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Find all header files recursively.
file(GLOB_RECURSE HEADER_FILES ${CMAKE_SOURCE_DIR}/src/*.h)

# Set the executable.
add_executable(${PROJECT_NAME} ${CPP_FILES} ${HEADER_FILES})

# Since the headers are also in /src we are adding it as an include directory.
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Copy the assets folder next to the executable after every build.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMAND ${CMAKE_COMMAND} -E echo
        "Copied assets to $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
)

# Copy the config folder next to the executable after every build.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/config"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/config"
        COMMAND ${CMAKE_COMMAND} -E echo
        "Copied config to $<TARGET_FILE_DIR:${PROJECT_NAME}>/config"
)

# Copy the scripts folder next to the executable after every build.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/scripts"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/scripts"
        COMMAND ${CMAKE_COMMAND} -E echo
        "Copied scripts to $<TARGET_FILE_DIR:${PROJECT_NAME}>/scripts"
)

# Find and link nlohmann-json.
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Find and link SDL3.
find_package(SDL3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# Find and link SDL3_image.
find_package(SDL3_image CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_image::SDL3_image)

# Find and link SDL3_ttf.
find_package(SDL3_ttf CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3_ttf::SDL3_ttf)

# Find and link box2d.
find_package(box2d CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE box2d::box2d)

# Find and link OpenGL + glad.
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL)

find_package(glad CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glad::glad)

# Find and link imgui
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)

# Find and link Lua + sol2.
find_package(Lua REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})

find_package(sol2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE sol2::sol2)

# sol2's template-heavy bindings exceed MSVC's default COFF section limit.
if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /bigobj)
endif ()

# Precompiled header.
target_precompile_headers(${PROJECT_NAME} PRIVATE
        # Standard library — slow parsers first.
        <filesystem>
        <format>
        <algorithm>
        <functional>
        <memory>
        <optional>
        <string>
        <string_view>
        <utility>
        <vector>
        <unordered_map>
        <unordered_set>
        <cstdint>

        # Graphics / windowing. glad must precede any GL-pulling header.
        <glad/glad.h>
        <SDL3/SDL.h>
        <SDL3_image/SDL_image.h>
        <SDL3_ttf/SDL_ttf.h>

        # UI.
        <imgui.h>
        <imgui_impl_sdl3.h>
        <imgui_impl_opengl3.h>

        # Physics.
        <box2d/box2d.h>

        # Scripting (heaviest single header in the project).
        <sol/sol.hpp>
)