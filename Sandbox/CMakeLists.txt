# Sandbox/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(Sandbox)

# First create the executable
add_executable(Sandbox src/main.cpp)

# Then link libraries
target_link_libraries(Sandbox PRIVATE Runtime)

# Fix the file copy command
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets/ 
         DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
         FILES_MATCHING PATTERN "*")
    message(STATUS "Assets copied to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets")
else()
    message(WARNING "Assets directory not found at ${CMAKE_CURRENT_SOURCE_DIR}/assets")
endif()