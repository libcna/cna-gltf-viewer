cmake_minimum_required(VERSION 3.21)

project(CnaGltfViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CNA_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cna" CACHE PATH
    "Path to the CNA framework checkout")

if(NOT EXISTS "${CNA_ROOT_DIR}/CMakeLists.txt")
    message(FATAL_ERROR
        "CNA was not found at '${CNA_ROOT_DIR}'.\n"
        "Clone https://github.com/openeggbert/cna next to this repository or set CNA_ROOT_DIR.")
endif()

set(CNA_GLTF_VIEWER_RENDERERS SDL_RENDERER OPENGLES3 BGFX VULKAN WEBGPU HEADLESS SOFTWARE DIRECTX11)
set(CNA_GRAPHICS_RENDERER "OPENGLES3" CACHE STRING "CNA graphics renderer")
set_property(CACHE CNA_GRAPHICS_RENDERER PROPERTY STRINGS ${CNA_GLTF_VIEWER_RENDERERS})

if(NOT CNA_GRAPHICS_RENDERER IN_LIST CNA_GLTF_VIEWER_RENDERERS)
    message(FATAL_ERROR
        "Unknown CNA_GRAPHICS_RENDERER='${CNA_GRAPHICS_RENDERER}'. "
        "Choose one of: ${CNA_GLTF_VIEWER_RENDERERS}.")
endif()

# The viewer needs CNA's CNAEXT PBR effects to faithfully display PBR glTF
# materials. CNA's own tests/examples and transitive dependency tests are not
# part of this application's build.
set(CNA_CNAEXT ON CACHE BOOL "Enable CNA extended graphics APIs" FORCE)
set(CNA_ENABLE_NET OFF CACHE BOOL "Disable unused CNA networking" FORCE)
set(CNA_BUILD_TESTS OFF CACHE BOOL "Disable CNA tests" FORCE)
set(CNA_BUILD_EXAMPLES OFF CACHE BOOL "Disable CNA examples" FORCE)
set(EASYGL_BUILD_TESTS OFF CACHE BOOL "Disable EasyGL tests" FORCE)
set(EASYGL_BUILD_EXAMPLES OFF CACHE BOOL "Disable EasyGL examples" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable transitive dependency tests" FORCE)

add_subdirectory("${CNA_ROOT_DIR}" CNA_BUILD)

if(NOT TARGET CNA)
    message(FATAL_ERROR
        "CNA did not define its compatible umbrella target for "
        "CNA_GRAPHICS_RENDERER=${CNA_GRAPHICS_RENDERER}.")
endif()

add_executable(cna_gltf_viewer
    src/CommandLine.cpp
    src/CnjConversion.cpp
    src/ViewerGame.cpp
    src/main.cpp
)

target_include_directories(cna_gltf_viewer PRIVATE src)

target_compile_definitions(cna_gltf_viewer PRIVATE
    CNA_GLTF_TO_CNJ_TOOL_PATH="$<TARGET_FILE:cna_tool_gltf_to_cnj>")

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cna_gltf_viewer PRIVATE -Wall -Wextra -Wpedantic)
endif()

# CNA's compatible umbrella already carries the selected renderer and declares
# the framework/renderer static-library cycles itself.
target_link_libraries(cna_gltf_viewer PRIVATE CNA)

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_mixer REQUIRED CONFIG)
target_link_libraries(cna_gltf_viewer PRIVATE SDL3::SDL3)

if(TARGET SDL3::SDL3main)
    target_link_libraries(cna_gltf_viewer PRIVATE SDL3::SDL3main)
endif()

if(MINGW)
    cna_copy_mingw_cxx_runtime(cna_gltf_viewer)
endif()
if(WIN32)
    cna_copy_sdl_runtime(cna_gltf_viewer)
endif()

add_dependencies(cna_gltf_viewer cna_tool_gltf_to_cnj)

option(CNA_GLTF_VIEWER_BUILD_TESTS "Build and register cna-gltf-viewer tests" ON)
if(CNA_GLTF_VIEWER_BUILD_TESTS)
    enable_testing()
    add_test(NAME CnaGltfViewerCliHelp COMMAND cna_gltf_viewer --help)
    add_test(
        NAME CnaGltfViewerConvertsTriangle
        COMMAND ${CMAKE_COMMAND}
            "-DCONVERTER=$<TARGET_FILE:cna_tool_gltf_to_cnj>"
            "-DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/tests/conversion"
            -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/VerifyConversion.cmake")

    # CNA's L0-L6 ladder is renderer-independent.  The production viewer supplies the final
    # OPENGLES3 rung: every canonical fixture is loaded through ContentManager in two independent
    # processes and compared with its committed EasyGL image/disposition oracle.
    if(CNA_GRAPHICS_RENDERER STREQUAL "OPENGLES3")
        find_package(Python3 REQUIRED COMPONENTS Interpreter)
        add_test(
            NAME CnaGltfConformanceL7
            COMMAND "${Python3_EXECUTABLE}"
                "${CNA_ROOT_DIR}/scripts/gltf-l7-corpus.py"
                --viewer "$<TARGET_FILE:cna_gltf_viewer>"
                --viewer-source "${CMAKE_CURRENT_SOURCE_DIR}")
        set_tests_properties(CnaGltfConformanceL7 PROPERTIES
            LABELS gltf-conformance
            TIMEOUT 600
            WORKING_DIRECTORY "${CNA_ROOT_DIR}")
    endif()
endif()
