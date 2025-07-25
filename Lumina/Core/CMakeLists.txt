cmake_minimum_required(VERSION 3.15)

# Core library sources and headers
set(CORE_SOURCES
    src/Engine.cpp
)

set(CORE_PUBLIC_HEADERS
    include/Core/Base.h
    include/Core/Defines.h
    include/Core/Engine.h
)

add_library(Core STATIC
    ${CORE_SOURCES}
    ${CORE_PUBLIC_HEADERS}
)

# Add GLM includes (header-only)
target_include_directories(Core PUBLIC
    ${CMAKE_SOURCE_DIR}/Lumina/Runtime/deps/glm
)

target_include_directories(Core
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        src
)

# Set output directory
set_target_properties(Core PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)