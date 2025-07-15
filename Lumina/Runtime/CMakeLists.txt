file(GLOB_RECURSE RUNTIME_SOURCES src/*.cpp)
file(GLOB_RECURSE RUNTIME_HEADERS include/*.h)

add_library(Runtime STATIC ${RUNTIME_SOURCES} ${RUNTIME_HEADERS})
target_include_directories(Runtime PUBLIC include)
target_link_libraries(Runtime PUBLIC Core)