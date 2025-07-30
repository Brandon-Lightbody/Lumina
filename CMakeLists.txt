cmake_minimum_required(VERSION 3.15)
project(Lumina VERSION 0.1.0)

# Set vcpkg toolchain and manifest mode
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
set(VCPKG_MANIFEST_MODE ON CACHE BOOL "Enable vcpkg manifest mode")

# Include platform detection and sanitizers
include(cmake/DetectSanitizer.cmake)
include(cmake/AsanHelpers.cmake)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type definitions
include(cmake/BuildType.cmake)

# Output directory config
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} ${CMAKE_BINARY_DIR}/bin)
endforeach()

# Add NRI directly
option(NRI_BUILD_BACKEND_D3D11  "Enable D3D11 Backend" OFF)
option(NRI_BUILD_BACKEND_D3D12  "Enable D3D12 Backend" ON)
option(NRI_BUILD_BACKEND_VULKAN "Enable Vulkan Backend" ON)

add_subdirectory(deps/NRI)

set(NRI_INCLUDE_DIRS
    ${CMAKE_SOURCE_DIR}/deps/NRI/Include
    ${CMAKE_BINARY_DIR}/deps/NRI/Include
    CACHE INTERNAL "NRI include paths"
)

# Add components
add_subdirectory(Lumina/Core)
add_subdirectory(Lumina/Runtime)
add_subdirectory(Sandbox)

# Set vcpkg manifest property globally
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_GLOBAL_VcpkgEnabled true)
set_property(GLOBAL PROPERTY USE_VCPKG_MANIFEST ON)

# Enable ASan for Debug on Windows
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND WIN32)
    add_link_options("$<$<CONFIG:Debug>:/fsanitize=address>")
endif()