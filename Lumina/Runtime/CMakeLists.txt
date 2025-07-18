cmake_minimum_required(VERSION 3.15)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(RUNTIME_SOURCES
    src/EntryPoint.cpp
    src/Application.cpp
    src/ScriptEngine.cpp
    src/ScriptAPI.cpp
)

set(RUNTIME_HEADERS
    include/Runtime/Application.h
    include/Runtime/EntryPoint.h
    include/Runtime/ScriptEngine.h
    include/Runtime/ScriptAPI.h
)

add_library(Runtime SHARED ${RUNTIME_SOURCES} ${RUNTIME_HEADERS})
target_compile_definitions(Runtime PRIVATE RUNTIME_EXPORTS)

target_include_directories(Runtime PUBLIC 
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/nethost
    ${CMAKE_SOURCE_DIR}/Lumina/Core/include
)

target_link_libraries(Runtime PUBLIC Core)

# Generate correct runtimeconfig
set(RUNTIMECONFIG_CONTENT
"{
  \"runtimeOptions\": {
    \"tfm\": \"net9.0\",
    \"rollForward\": \"latestMinor\",
    \"framework\": {
      \"name\": \"Microsoft.NETCore.App\",
      \"version\": \"9.0.0\"
    }
  }
}")

# Always write the correct runtimeconfig
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/scriptapi/ScriptAPI.runtimeconfig.json" ${RUNTIMECONFIG_CONTENT})

# Build managed assembly with correct output path
add_custom_target(build_scriptapi_assembly
    COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/scriptapi -c $<CONFIG> -o ${CMAKE_BINARY_DIR}/bin
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scriptapi
    COMMENT "Building managed assembly"
)

add_dependencies(Runtime build_scriptapi_assembly)

# Copy all managed artifacts
add_custom_command(TARGET Runtime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/scriptapi/ScriptAPI.runtimeconfig.json"
        "${CMAKE_BINARY_DIR}/bin/ScriptAPI.runtimeconfig.json"
    COMMENT "Copying managed dependencies to output directory"
)

if(WIN32)
    target_link_libraries(Runtime PRIVATE Shlwapi.lib)
endif()

set_target_properties(Runtime PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)