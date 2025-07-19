# Base flags
set(MY_FLAGS "-Wall -Wextra -Wpedantic -Wshadow -Werror")
set(MY_FLAGS_DEBUG "-fsanitize=address,undefined -fno-omit-frame-pointer -O2")
set(MY_FLAGS_RELEASE "-O3 -DNDEBUG")

# Set init flags
set(CMAKE_C_FLAGS_INIT "${MY_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${MY_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG_INIT "${MY_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "${MY_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE_INIT "${MY_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "${MY_FLAGS_RELEASE}")

# Include vcpkg
include(${CMAKE_CURRENT_LIST_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake)

# Handle MSVC specifics
if(MSVC)
    add_compile_options(/Zi)
    add_link_options(/DEBUG)
endif()