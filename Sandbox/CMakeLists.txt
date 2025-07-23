add_executable(Sandbox "src/Sandbox.cpp")
target_link_libraries(Sandbox PRIVATE Runtime)

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    copy_msvc_asan_dll(Sandbox)
endif()

# Ensure managed artifacts are copied to Sandbox output
add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE_DIR:Runtime>/ScriptAPI.runtimeconfig.json"
        "$<TARGET_FILE_DIR:Sandbox>/ScriptAPI.runtimeconfig.json"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE_DIR:Runtime>/ScriptAPI.dll"
        "$<TARGET_FILE_DIR:Sandbox>/ScriptAPI.dll"
    COMMENT "Copying managed artifacts to Sandbox output"
)