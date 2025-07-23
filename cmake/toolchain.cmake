# Only apply to debug builds
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  return()
endif()

# Add version check
if(MSVC AND MSVC_VERSION LESS 1929)
  message(WARNING "MSVC ASan requires VS 2019 v16.9+. Disabling ASan.")
  return()
endif()

# Compiler-specific sanitizer configuration
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # MSVC native ASan
  add_compile_options(/fsanitize=address /Zi /Oy-)
  add_link_options(/DEBUG:FASTLINK /INCREMENTAL:NO)
  
  # Workaround for debugger integration
  add_compile_definitions(
    _DISABLE_STRING_ANNOTATION 
    _DISABLE_VECTOR_ANNOTATION
    _ITERATOR_DEBUG_LEVEL=0
    _CRT_NOFORCE_MANIFEST
  )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Clang or clang-cl sanitizer flags
  if(MSVC)
    # clang-cl (Windows)
    add_compile_options(
      /fsanitize=address,undefined
      /fno-omit-frame-pointer
    )
    add_link_options(
      /fsanitize=address,undefined
    )
  else()
    # Standard Clang (Linux/macOS)
    add_compile_options(
      -fsanitize=address,undefined
      -fno-omit-frame-pointer
    )
    add_link_options(-fsanitize=address,undefined)
  endif()
endif()

# Include vcpkg AFTER setting flags
include("${CMAKE_CURRENT_LIST_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" OPTIONAL)