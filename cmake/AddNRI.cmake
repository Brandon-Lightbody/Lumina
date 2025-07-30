# cmake/AddNRI.cmake

set(NRI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dep/NRI")

if(NOT EXISTS "${NRI_SOURCE_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "NRI not found at ${NRI_SOURCE_DIR}")
endif()

# Optional: choose only the backends you want
option(NRI_BUILD_BACKEND_D3D11  "Enable D3D11 Backend" OFF)
option(NRI_BUILD_BACKEND_D3D12  "Enable D3D12 Backend" ON)
option(NRI_BUILD_BACKEND_VULKAN "Enable Vulkan Backend" ON)

# Add NRI as a subproject
add_subdirectory(${NRI_SOURCE_DIR} ${CMAKE_BINARY_DIR}/_deps/NRI)

# Add alias for convenience
add_library(NRI::Core ALIAS NRI)
