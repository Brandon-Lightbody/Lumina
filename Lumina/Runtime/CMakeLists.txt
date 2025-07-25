cmake_minimum_required(VERSION 3.15)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

# NRI configuration
set(NRI_SOURCE_DIR "deps/NRI/Source")
set(NRI_INCLUDE_DIR "deps/NRI/Include")

# Add NRI library
add_library(NRI STATIC
    "${NRI_SOURCE_DIR}/Extensions/Extensions.cpp"
    "${NRI_SOURCE_DIR}/Extensions/HelperFunctions.cpp"
    "${NRI_SOURCE_DIR}/Extensions/WrapperD3D11.cpp"
    "${NRI_SOURCE_DIR}/Extensions/WrapperD3D12.cpp"
    "${NRI_SOURCE_DIR}/Extensions/WrapperVK.cpp"
    "${NRI_SOURCE_DIR}/NRI.cpp"
)

target_include_directories(NRI PUBLIC ${NRI_INCLUDE_DIR})
target_compile_definitions(NRI PRIVATE _CRT_SECURE_NO_WARNINGS)

# Renderer sources (unchanged)
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

# Renderer headers (unchanged)
set(RENDERER_HEADERS
    src/Renderer/Core/Renderer.h
    src/Renderer/Core/RendererAPI.h
    src/Renderer/Core/FrameGraph.h
    src/Renderer/Resources/ResourceManager.h
    src/Renderer/Resources/Texture.h
    src/Renderer/Resources/Buffer.h
    src/Renderer/Lighting/Lighting.h
    src/Renderer/Lighting/ClusteredShading.h
    src/Renderer/Lighting/Shadows.h
    src/Renderer/RayTracing/RayTracingCore.h
    src/Renderer/RayTracing/AccelerationStructure.h
    src/Renderer/RayTracing/HybridRendering.h
    src/Renderer/PostProcess/PostProcessing.h
    src/Renderer/PostProcess/Bloom.h
    src/Renderer/PostProcess/ToneMapping.h
    src/Renderer/Text/TextRenderer.h
    src/Renderer/Text/FontAtlas.h
    src/Renderer/Text/SDFGenerator.h
    src/Renderer/Debug/DebugRenderer.h
    src/Renderer/Debug/Gizmos.h
    src/Renderer/Shading/MaterialSystem.h
    src/Renderer/Shading/ShaderSystem.h
    src/Renderer/Rendering/InstancedRenderer.h
    src/Renderer/Rendering/Mesh.h
    src/Renderer/Rendering/RenderQueue.h
    src/Renderer/Shaders/ShaderCompiler.h
    src/Renderer/Shaders/ShaderReflection.h
    src/Renderer/Scene/Camera.h
    src/Renderer/Scene/Scene.h
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
)

add_library(Runtime SHARED ${RUNTIME_SOURCES} ${RUNTIME_HEADERS} ${RENDERER_HEADERS})
target_compile_definitions(Runtime PRIVATE RUNTIME_EXPORTS)

# Add GLM includes (header-only)
target_include_directories(Runtime PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm
)

target_include_directories(Runtime
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${CMAKE_SOURCE_DIR}/Lumina/Core/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/nethost
        ${NRI_INCLUDE_DIR}
)

# FIXED: Removed glm::glm linking
target_link_libraries(Runtime PUBLIC Core NRI)

# Generate runtimeconfig (unchanged)
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

file(WRITE "${CMAKE_BINARY_DIR}/ScriptAPI.runtimeconfig.json" ${RUNTIMECONFIG_CONTENT})

# Build managed assembly (unchanged)
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/bin/ScriptAPI.dll"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/bin"
  COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}/ScriptAPI" -c $<CONFIG> -o "${CMAKE_BINARY_DIR}/bin"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ScriptAPI/ScriptAPI.csproj"
  COMMENT "Building managed assembly"
)

add_custom_target(build_scriptapi_assembly ALL
  DEPENDS "${CMAKE_BINARY_DIR}/bin/ScriptAPI.dll"
)

add_dependencies(Runtime build_scriptapi_assembly)

# Copy managed artifacts (unchanged)
add_custom_command(TARGET Runtime POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/ScriptAPI.runtimeconfig.json"
    "$<TARGET_FILE_DIR:Runtime>/ScriptAPI.runtimeconfig.json"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/ScriptAPI.dll"
    "$<TARGET_FILE_DIR:Runtime>/ScriptAPI.dll"
  COMMENT "Copying managed artifacts"
)

# Add NRI dependencies (unchanged)
if(WIN32)
    target_link_libraries(NRI PRIVATE 
        d3d11.lib
        d3d12.lib
        dxgi.lib
    )
    target_link_libraries(Runtime PRIVATE Shlwapi.lib)
endif()

set_target_properties(Runtime PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

if(MSVC)
    set_target_properties(Runtime PROPERTIES
        LINK_FLAGS "/ignore:4197"
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(WIN32 AND MSVC)
        # ASan workarounds
        target_compile_options(Runtime PRIVATE "/Zc:threadSafeInit-")
        target_compile_definitions(Runtime PRIVATE _DISABLE_DEPRECATE_STATIC_CPPLIB)
    endif()
endif()

# Vulkan support (unchanged)
find_package(Vulkan)
if(Vulkan_FOUND)
    target_include_directories(NRI PRIVATE ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(NRI PRIVATE ${Vulkan_LIBRARIES})
endif()