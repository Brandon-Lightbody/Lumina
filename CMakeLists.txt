cmake_minimum_required(VERSION 3.21)
project(Lumina)

set(CMAKE_CXX_STANDARD 17)  # Default to C++17
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable vcpkg if available
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
endif()

# Define the executable
add_executable(Lumina ${CMAKE_CURRENT_SOURCE_DIR}/Lumina/Lumina.cpp)

# Set C++20 for this target (if needed)
if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET Lumina PROPERTY CXX_STANDARD 20)
endif()

# Find and link dependencies
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

target_link_libraries(Lumina PRIVATE glfw glm::glm Vulkan::Vulkan)
target_include_directories(Lumina PRIVATE ${Vulkan_INCLUDE_DIR})

# Include subdirectory (if needed for other reasons)
add_subdirectory(Lumina)