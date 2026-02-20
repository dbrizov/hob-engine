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

# Find and link SDL2.
find_package(SDL2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>, SDL2::SDL2, SDL2::SDL2-static>
)

# Find and link SDL2_image.
find_package(SDL2_image CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>, SDL2_image::SDL2_image, SDL2_image::SDL2_image-static>
)

# Find and link SDL2_ttf.
find_package(SDL2_ttf CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>, SDL2_ttf::SDL2_ttf, SDL2_ttf::SDL2_ttf-static>
)

# Find and link box2d.
find_package(box2d CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE box2d::box2d)

# Find and link nlohmann-json.
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# Find and link fmt.
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

# Find and link imgui
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui)