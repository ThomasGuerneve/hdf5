# Copyright (c) 2013, 2018 Ruslan Baratov All rights reserved.

if(DEFINED POLLY_FLAGS_VS_WARNINGS_CMAKE_)
    return()
else()
    set(POLLY_FLAGS_VS_WARNINGS_CMAKE_ 1)
endif()

include(polly_add_cache_flag)

polly_add_cache_flag(CMAKE_C_FLAGS "-D_SCL_SECURE_NO_WARNINGS")
polly_add_cache_flag(CMAKE_C_FLAGS "-D_CRT_SECURE_NO_WARNINGS")
polly_add_cache_flag(CMAKE_CXX_FLAGS "-D_SCL_SECURE_NO_WARNINGS")
polly_add_cache_flag(CMAKE_CXX_FLAGS "-D_CRT_SECURE_NO_WARNINGS")
