set(RUNTIME_SOURCES
    src/EntryPoint.cpp
    src/Application.cpp
)

set(RUNTIME_HEADERS
    include/Runtime/Application.h
    include/Runtime/EntryPoint.h
)

add_library(Runtime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS})
target_include_directories(Runtime PUBLIC include)
target_link_libraries(Runtime PUBLIC Core)