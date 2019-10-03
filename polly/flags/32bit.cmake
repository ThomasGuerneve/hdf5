# Copyright (c) 2017, Ruslan Baratov
# All rights reserved.

if(DEFINED POLLY_FLAGS_32BIT_CMAKE)
  return()
else()
  set(POLLY_FLAGS_32BIT_CMAKE 1)
endif()

include(polly_add_cache_flag)

polly_add_cache_flag(CMAKE_CXX_FLAGS "-m32")
polly_add_cache_flag(CMAKE_C_FLAGS "-m32")

polly_add_cache_flag(CMAKE_NVCC_FLAGS "-m32")

set(HUNTER_OPENSSL_ARCHITECTURE "linux-generic32")

if (NOT CMAKE_SYSTEM_PROCESSOR)
    execute_process(
        COMMAND linux32 uname -m
        OUTPUT_VARIABLE POLLY_SYSTEM_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (POLLY_SYSTEM_ARCH)
        set(CMAKE_SYSTEM_PROCESSOR ${POLLY_SYSTEM_ARCH})
        set(CMAKE_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME})

        # Set emulator if it is unset
        if (NOT CMAKE_CROSSCOMPILING_EMULATOR)
            set(CMAKE_CROSSCOMPILING_EMULATOR linux32)
        endif()
    endif()
endif()

list(APPEND HUNTER_TOOLCHAIN_UNDETECTABLE_ID "m32")
