find_package(Catch2 REQUIRED CONFIG)

file(GLOB_RECURSE TEST_SOURCES src/*.cpp src/*.h)
add_executable(Tests ${TEST_SOURCES})
target_link_libraries(Tests PRIVATE Runtime Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(Tests)