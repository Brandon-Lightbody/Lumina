# Build as SHARED library
add_library(ScriptAPI SHARED 
    src/ScriptEngine.cpp 
    src/ScriptAPI.cpp
)

# Add export definition
target_compile_definitions(ScriptAPI PRIVATE SCRIPTAPI_EXPORTS)

# Add CoreCLR headers
target_include_directories(ScriptAPI PUBLIC 
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/nethost
)

# Build managed assembly
add_custom_target(build_managed_assembly
    COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/managed -c $<CONFIG> -o ${CMAKE_BINARY_DIR}/bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/managed
    COMMENT "Building managed assembly"
)

# Link dependencies
target_link_libraries(ScriptAPI PUBLIC Core Runtime)

# Add dependency on managed assembly
add_dependencies(ScriptAPI build_managed_assembly)

# Copy managed files to output directory
add_custom_command(TARGET ScriptAPI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/managed/ManagedApp.runtimeconfig.json
        $<TARGET_FILE_DIR:ScriptAPI>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/bin/ManagedApp.dll
        $<TARGET_FILE_DIR:ScriptAPI>
    COMMENT "Copying managed dependencies to output directory"
)

# Link Shlwapi on Windows
if(WIN32)
    target_link_libraries(ScriptAPI PRIVATE Shlwapi.lib)
endif()