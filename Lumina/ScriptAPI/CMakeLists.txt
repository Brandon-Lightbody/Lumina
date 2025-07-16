# Build as STATIC library
add_library(ScriptAPI STATIC src/ScriptEngine.cpp)

# Add CoreCLR headers and build directory
target_include_directories(ScriptAPI PUBLIC 
    include
    deps/nethost
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Build managed assembly
add_custom_target(build_managed_assembly
    COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/managed -c $<CONFIG> -o ${CMAKE_CURRENT_BINARY_DIR}/managed
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/managed
    COMMENT "Building managed assembly"
)

# Generate embedded assembly header
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedAssembly.h
    COMMAND ${CMAKE_COMMAND} 
        -DINPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/managed/ManagedApp.dll"
        -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/EmbeddedAssembly.h
        -P ${CMAKE_SOURCE_DIR}/cmake/EmbedAssembly.cmake
    DEPENDS build_managed_assembly
    COMMENT "Embedding managed assembly"
)

# Add generated header to sources
target_sources(ScriptAPI PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/EmbeddedAssembly.h
)

# Link dependencies
target_link_libraries(ScriptAPI PUBLIC Core Runtime)

# Add dependency on managed assembly
add_dependencies(ScriptAPI build_managed_assembly)

# Copy runtime config
add_custom_command(TARGET build_managed_assembly POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/managed/ManagedApp.runtimeconfig.json
        ${CMAKE_CURRENT_BINARY_DIR}/managed
    COMMENT "Copying runtime config"
)

# Link Shlwapi on Windows
if(WIN32)
    target_link_libraries(ScriptAPI PRIVATE Shlwapi.lib)
endif()