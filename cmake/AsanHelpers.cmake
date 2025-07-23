function(copy_msvc_asan_dll target)
  if(MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Improved compiler detection
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC|Intel")
      set(ASAN_DLL_NAME "clang_rt.asan_dbg_dynamic-x86_64.dll")
      set(ASAN_LIB_NAME "clang_rt.asan_dbg_dynamic-x86_64.lib")
    else()
      set(ASAN_DLL_NAME "clang_rt.asan_dynamic-x86_64.dll")
      set(ASAN_LIB_NAME "clang_rt.asan_dynamic-x86_64.lib")
    endif()
    
    # Expanded search paths
    get_filename_component(COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(SEARCH_PATHS
      "${COMPILER_DIR}"
      "${COMPILER_DIR}/../bin/Host${CMAKE_HOST_SYSTEM_PROCESSOR}/${CMAKE_SYSTEM_PROCESSOR}"
      "${COMPILER_DIR}/../lib/clang/${CMAKE_CXX_COMPILER_VERSION}/lib/windows"
      "$ENV{VCINSTALLDIR}/bin/Host${CMAKE_HOST_SYSTEM_PROCESSOR}/${CMAKE_SYSTEM_PROCESSOR}"
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Enterprise/VC/Redist/MSVC"
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Professional/VC/Redist/MSVC"
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC"
      "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio/2022/Enterprise/VC/Redist/MSVC"
      "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio/2022/Professional/VC/Redist/MSVC"
      "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio/2022/Community/VC/Redist/MSVC"
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2019/Enterprise/VC/Redist/MSVC"
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2019/Professional/VC/Redist/MSVC"
      "$ENV{ProgramFiles}/Microsoft Visual Studio/2019/Community/VC/Redist/MSVC"
      "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio/2019/Enterprise/VC/Redist/MSVC"
      "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio/2019/Professional/VC/Redist/MSVC"
      "$ENV{ProgramFiles} (x86)/Microsoft Visual Studio/2019/Community/VC/Redist/MSVC"
      "$ENV{SystemDrive}/Windows/System32"
    )
    
    # Find ASan DLL
    find_file(ASAN_DLL
      NAMES ${ASAN_DLL_NAME}
      PATHS ${SEARCH_PATHS}
      NO_DEFAULT_PATH
    )
    
    # Find ASan LIB
    find_library(ASAN_LIB
      NAMES ${ASAN_LIB_NAME}
      PATHS ${SEARCH_PATHS}
      NO_DEFAULT_PATH
    )
    
    if(ASAN_DLL)
      message(STATUS "Found ASan DLL: ${ASAN_DLL}")
      
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${ASAN_DLL}"
          "$<TARGET_FILE_DIR:${target}>"
        COMMENT "Copying ASan DLL: ${ASAN_DLL_NAME}"
      )
    else()
      message(WARNING "ASan DLL not found. Searched in: ${SEARCH_PATHS}")
    endif()
  endif()
endfunction()