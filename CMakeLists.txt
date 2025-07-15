cmake_minimum_required(VERSION 3.15)
project(Lumina VERSION 0.1.0)

# Set build types
include(cmake/BuildType.cmake)

# Output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Add components
add_subdirectory(Lumina/Core)
add_subdirectory(Lumina/Runtime)
add_subdirectory(Lumina/ScriptAPI)
add_subdirectory(Lumina/Tests)
add_subdirectory(Sandbox)