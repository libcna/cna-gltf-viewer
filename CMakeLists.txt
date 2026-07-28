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

set(CNA_GLTF_VIEWER_BACKENDS SDL_RENDERER EASYGL BGFX VULKAN WEBGPU HEADLESS SOFTWARE)
set(CNA_GRAPHICS_BACKEND "EASYGL" CACHE STRING "CNA graphics backend")
set_property(CACHE CNA_GRAPHICS_BACKEND PROPERTY STRINGS ${CNA_GLTF_VIEWER_BACKENDS})

if(NOT CNA_GRAPHICS_BACKEND IN_LIST CNA_GLTF_VIEWER_BACKENDS)
    message(FATAL_ERROR
        "Unknown CNA_GRAPHICS_BACKEND='${CNA_GRAPHICS_BACKEND}'. "
        "Choose one of: ${CNA_GLTF_VIEWER_BACKENDS}.")
endif()

# The viewer needs CNA's NOXNA PBR effects to faithfully display PBR glTF
# materials. CNA's own tests/examples and transitive dependency tests are not
# part of this application's build.
set(CNA_NOXNA ON CACHE BOOL "Enable CNA extended graphics APIs" FORCE)
set(CNA_ENABLE_NET OFF CACHE BOOL "Disable unused CNA networking" FORCE)
set(CNA_BUILD_TESTS OFF CACHE BOOL "Disable CNA tests" FORCE)
set(CNA_BUILD_EXAMPLES OFF CACHE BOOL "Disable CNA examples" FORCE)
set(EASYGL_BUILD_TESTS OFF CACHE BOOL "Disable EasyGL tests" FORCE)
set(EASYGL_BUILD_EXAMPLES OFF CACHE BOOL "Disable EasyGL examples" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "Disable transitive dependency tests" FORCE)

add_subdirectory("${CNA_ROOT_DIR}" CNA_BUILD)

string(TOLOWER "${CNA_GRAPHICS_BACKEND}" CNA_GLTF_VIEWER_BACKEND_LOWER)
set(CNA_GLTF_VIEWER_BACKEND_TARGET
    "cna_backend_graphics_${CNA_GLTF_VIEWER_BACKEND_LOWER}")

if(NOT TARGET ${CNA_GLTF_VIEWER_BACKEND_TARGET})
    message(FATAL_ERROR
        "CNA did not define '${CNA_GLTF_VIEWER_BACKEND_TARGET}' for "
        "CNA_GRAPHICS_BACKEND=${CNA_GRAPHICS_BACKEND}.")
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

# CNA and its selected static graphics backend refer to one another on GNU ld.
# Keep both targets in a linker group, matching CNA's own consumer templates.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32)
    target_link_libraries(cna_gltf_viewer PRIVATE
        -Wl,--start-group CNA ${CNA_GLTF_VIEWER_BACKEND_TARGET} -Wl,--end-group)
else()
    target_link_libraries(cna_gltf_viewer PRIVATE CNA ${CNA_GLTF_VIEWER_BACKEND_TARGET})
endif()

find_package(SDL3 REQUIRED CONFIG)
target_link_libraries(cna_gltf_viewer PRIVATE SDL3::SDL3)

if(TARGET SDL3::SDL3main)
    target_link_libraries(cna_gltf_viewer PRIVATE SDL3::SDL3main)
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
endif()
