cmake_minimum_required(VERSION 3.80)
project(voxel_lab LANGUAGES CXX)

option(BUILD_DEPS "Fetch third-party dependencies automatically" ON)
option(BUILD_TESTING "Build tests" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)
find_package(OpenGL REQUIRED)  # for OpenGL::GL target

# ----------------------------
# Dependencies (SDL2, GLEW, GLM, ImGui, Catch2)
# ----------------------------
if (BUILD_DEPS)
    # SDL2 (official repo has modern CMake)
    FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-2.30.8
    GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(SDL2)

    # GLEW via CMake-friendly fork (defines libglew_static / libglew_shared)
    FetchContent_Declare(glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG glew-cmake-2.2.0
    GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(glew)

    # Provide the alias the rest of the project expects
    if (TARGET libglew_static)
        add_library(GLEW::GLEW ALIAS libglew_static)
        # Needed when linking against the static target
        set_property(TARGET libglew_static PROPERTY INTERFACE_COMPILE_DEFINITIONS GLEW_STATIC)
    elseif (TARGET libglew_shared)
        add_library(GLEW::GLEW ALIAS libglew_shared)
    else()
        message(FATAL_ERROR "GLEW target not found after FetchContent(glew)")
    endif()

    # GLM
    FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(glm)

    # Dear ImGui
    FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0
    GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(imgui)

    # Catch2 (tests)
    if (BUILD_TESTING)
        FetchContent_Declare(Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.5.2
      GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(Catch2)
    endif()
else()
    # Use system packages
    find_package(SDL2 REQUIRED CONFIG)
    find_package(GLEW REQUIRED)  # provides GLEW::GLEW imported target. See CMake docs.
    find_package(glm REQUIRED CONFIG)
    # For ImGui in system mode, either package it yourself or set IMGUI_DIR; here we fetch it anyway for convenience:
    FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.0
    GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(imgui)
    if (BUILD_TESTING)
        find_package(Catch2 3 REQUIRED)
    endif()
endif()

# ----------------------------
# ImGui wrapper target
# ----------------------------
add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${imgui_SOURCE_DIR}/misc/cpp
)
# Use GLEW as the OpenGL function loader in the ImGui backend
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(imgui PUBLIC SDL2::SDL2 GLEW::GLEW OpenGL::GL)

# ----------------------------
# App
# ----------------------------
file(GLOB APP_SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_executable(voxel_lab ${APP_SOURCES}
  shaders/cube.vert
  shaders/cube.frag
  resources/menus.txt
)
target_include_directories(voxel_lab PRIVATE src)
target_link_libraries(voxel_lab PRIVATE SDL2::SDL2 GLEW::GLEW glm::glm imgui OpenGL::GL)

# Embed default resource dirs at compile time
target_compile_definitions(voxel_lab PRIVATE
  DEFAULT_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
  DEFAULT_RES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
)

# Warnings
if (MSVC)
    target_compile_options(voxel_lab PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(voxel_lab PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ----------------------------
# Tests
# ----------------------------
if (BUILD_TESTING)
    enable_testing()
    add_executable(voxel_lab_tests
    tests/test_universe.cpp
    tests/test_commands.cpp
    src/universe.cpp src/selection.cpp src/commands.cpp src/util.hpp
  )
    target_include_directories(voxel_lab_tests PRIVATE src)
    target_link_libraries(voxel_lab_tests PRIVATE glm::glm)
    if (TARGET Catch2::Catch2WithMain)
        target_link_libraries(voxel_lab_tests PRIVATE Catch2::Catch2WithMain)
    else()
        find_package(Catch2 3 REQUIRED)
        target_link_libraries(voxel_lab_tests PRIVATE Catch2::Catch2WithMain)
    endif()
    add_test(NAME voxel_lab_tests COMMAND voxel_lab_tests)
endif()
