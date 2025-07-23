cmake_minimum_required(VERSION 3.15)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

set(RENDERER_SOURCES
    src/Renderer/Core/Renderer.cpp
    src/Renderer/Core/RendererAPI.cpp
    src/Renderer/Core/FrameGraph.cpp
    src/Renderer/NRI/NRIRenderer.cpp
    src/Renderer/Resources/ResourceManager.cpp
    src/Renderer/Resources/Texture.cpp
    src/Renderer/Resources/Buffer.cpp
    src/Renderer/Lighting/Lighting.cpp
    src/Renderer/Lighting/ClusteredShading.cpp
    src/Renderer/Lighting/Shadows.cpp
    src/Renderer/RayTracing/RayTracingCore.cpp
    src/Renderer/RayTracing/AccelerationStructure.cpp
    src/Renderer/RayTracing/HybridRendering.cpp
    src/Renderer/PostProcess/PostProcessing.cpp
    src/Renderer/PostProcess/Bloom.cpp
    src/Renderer/PostProcess/ToneMapping.cpp
    src/Renderer/Text/TextRenderer.cpp
    src/Renderer/Text/FontAtlas.cpp
    src/Renderer/Text/SDFGenerator.cpp
    src/Renderer/Debug/DebugRenderer.cpp
    src/Renderer/Debug/Gizmos.cpp
    src/Renderer/Shading/MaterialSystem.cpp
    src/Renderer/Shading/ShaderSystem.cpp
    src/Renderer/Rendering/InstancedRenderer.cpp
    src/Renderer/Rendering/Mesh.cpp
    src/Renderer/Rendering/RenderQueue.cpp
    src/Renderer/Shaders/ShaderCompiler.cpp
    src/Renderer/Shaders/ShaderReflection.cpp
    src/Renderer/Scene/Camera.cpp
    src/Renderer/Scene/Scene.cpp
)

set(RENDERER_HEADERS
    include/Runtime/Renderer/Renderer.h
    include/Runtime/Renderer/RendererAPI.h
    include/Runtime/Renderer/FrameGraph.h
    include/Runtime/Renderer/ResourceManager.h
    include/Runtime/Renderer/Texture.h
    include/Runtime/Renderer/Buffer.h
    include/Runtime/Renderer/Lighting.h
    include/Runtime/Renderer/ClusteredShading.h
    include/Runtime/Renderer/Shadows.h
    include/Runtime/Renderer/RayTracingCore.h
    include/Runtime/Renderer/AccelerationStructure.h
    include/Runtime/Renderer/HybridRendering.h
    include/Runtime/Renderer/PostProcessing.h
    include/Runtime/Renderer/Bloom.h
    include/Runtime/Renderer/ToneMapping.h
    include/Runtime/Renderer/TextRenderer.h
    include/Runtime/Renderer/FontAtlas.h
    include/Runtime/Renderer/SDFGenerator.h
    include/Runtime/Renderer/DebugRenderer.h
    include/Runtime/Renderer/Gizmos.h
    include/Runtime/Renderer/MaterialSystem.h
    include/Runtime/Renderer/ShaderSystem.h
    include/Runtime/Renderer/InstancedRenderer.h
    include/Runtime/Renderer/Mesh.h
    include/Runtime/Renderer/RenderQueue.h
    include/Runtime/Renderer/ShaderCompiler.h
    include/Runtime/Renderer/ShaderReflection.h
    include/Runtime/Renderer/Camera.h
    include/Runtime/Renderer/Scene.h
)

set(RUNTIME_SOURCES
    src/EntryPoint.cpp
    src/Application.cpp
    src/ScriptEngine.cpp
    src/ScriptAPI.cpp
    ${RENDERER_SOURCES}
)

set(RUNTIME_HEADERS
    include/Runtime/Application.h
    include/Runtime/EntryPoint.h
    include/Runtime/RuntimeExports.h
    include/Runtime/ScriptAPI.h
    include/Runtime/ScriptEngine.h
    ${RENDERER_HEADERS}
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

# Generate runtimeconfig
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

# Write runtimeconfig to build directory
file(WRITE "${CMAKE_BINARY_DIR}/ScriptAPI.runtimeconfig.json" ${RUNTIMECONFIG_CONTENT})

# Build managed assembly with correct output path
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/ScriptAPI.dll"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
  COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/scriptapi -c $<CONFIG> -o "${CMAKE_BINARY_DIR}/bin"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/scriptapi/ScriptAPI.csproj"
  COMMENT "Building managed assembly"
)

add_custom_target(build_scriptapi_assembly ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/ScriptAPI.dll"
)

add_dependencies(Runtime build_scriptapi_assembly)

# Copy all managed artifacts
add_custom_command(TARGET Runtime POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/ScriptAPI.runtimeconfig.json"
    "$<TARGET_FILE_DIR:Runtime>/ScriptAPI.runtimeconfig.json"
  COMMENT "Copying managed configuration"
)

if(WIN32)
    target_link_libraries(Runtime PRIVATE Shlwapi.lib)
endif()

set_target_properties(Runtime PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Suppress linker warnings about duplicate exports
if(MSVC)
    set_target_properties(Runtime PROPERTIES
        LINK_FLAGS "/ignore:4197"
    )
endif()

# ASan and memory management
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Exclude managed .NET components from sanitizers
    set_target_properties(build_scriptapi_assembly PROPERTIES 
        COMPILE_FLAGS ""
        LINK_FLAGS ""
    )
    
    # Add ASan DLL copy for Windows
    if(WIN32)
        copy_msvc_asan_dll(Runtime)
        
        # Workaround for VS debugger
        if(MSVC)
            target_compile_options(Runtime PRIVATE "/Zc:threadSafeInit-")
            target_compile_definitions(Runtime PRIVATE _DISABLE_DEPRECATE_STATIC_CPPLIB)
        endif()
    endif()
endif()