# Runtime library
add_library(Runtime STATIC)

target_link_libraries(Runtime PUBLIC
    Core
    Platform
)

# Sources
target_sources(Runtime PRIVATE
    src/Application.cpp
)

# Public API
target_include_directories(Runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)