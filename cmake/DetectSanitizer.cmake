function(check_sanitizer_support)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Check if sanitizers are supported
        include(CheckCXXCompilerFlag)
        check_cxx_compiler_flag("-fsanitize=address" COMPILER_SUPPORTS_ASAN)
        
        if(NOT COMPILER_SUPPORTS_ASAN)
            message(WARNING "Compiler doesn't support AddressSanitizer")
        endif()
    elseif(MSVC)
        # Enhanced MSVC ASan check
        if(MSVC_VERSION GREATER_EQUAL 1929)  # VS 2019 v16.9+
            # Check if ASan DLL exists
            if(NOT ASAN_DLL)
                message(STATUS "MSVC ASan: supported but DLL not found yet")
            else()
                message(STATUS "MSVC ASan: fully supported")
            endif()
        else()
            message(WARNING "MSVC ASan requires Visual Studio 2019 version 16.9 or later")
        endif()
    endif()
endfunction()

check_sanitizer_support()