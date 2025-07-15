add_library(ScriptAPI SHARED src/ScriptEngine.cpp)
target_include_directories(ScriptAPI PUBLIC include)
target_link_libraries(ScriptAPI PUBLIC Core Runtime)

# Export symbols
target_compile_definitions(ScriptAPI PRIVATE SCRIPTAPI_EXPORTS)
set_target_properties(ScriptAPI PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CXX_VISIBILITY_PRESET hidden
)

# Build managed code
add_custom_command(TARGET ScriptAPI POST_BUILD
    COMMAND dotnet build ${CMAKE_CURRENT_SOURCE_DIR}/managed -c $<CONFIG> -o $<TARGET_FILE_DIR:ScriptAPI>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/managed
    COMMENT "Building managed assembly"
    VERBATIM
)