add_executable(Sandbox src/SandboxApp.cpp)
target_link_libraries(Sandbox PRIVATE Core Runtime ScriptAPI)

# Export functions for P/Invoke
if(WIN32)
    set_target_properties(Sandbox PROPERTIES
        ENABLE_EXPORTS ON
        LINK_FLAGS "/EXPORT:CreateString"
    )
else()
    target_link_options(Sandbox PRIVATE "LINKER:--export-dynamic")
endif()