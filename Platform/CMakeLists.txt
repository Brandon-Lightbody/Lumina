# Platform implementation library
add_library(Platform STATIC)

# Set include directories
target_include_directories(Platform PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Find packages
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(Platform PUBLIC
    Core
    glfw
)

# Platform implementations
target_sources(Platform PRIVATE
    src/GLFW/GLFWWindow.cpp
    src/GLFW/GLFWWindowFactory.cpp
)

# Configuration
target_compile_definitions(Platform PRIVATE
    GLFW_INCLUDE_NONE
)