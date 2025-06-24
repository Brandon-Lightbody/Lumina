add_library(Core STATIC)

# Public API headers
target_include_directories(Core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Source files
target_sources(Core PRIVATE
    src/Log.cpp
    src/Window.cpp
)

# Dependencies
find_package(fmt CONFIG REQUIRED)

target_link_libraries(Core PUBLIC
    fmt::fmt
)