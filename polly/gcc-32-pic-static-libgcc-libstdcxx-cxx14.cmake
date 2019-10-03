# Copyright (c) 2015, Ruslan Baratov
# All rights reserved.

if(DEFINED POLLY_GCC32_PIC_STATIC_LIBGCC_LIBSTDCXX_CMAKE_)
  return()
else()
  set(POLLY_GCC32_PIC_STATIC_LIBGCC_LIBSTDCXX_CMAKE_ 1)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_init.cmake")

polly_init(
    "gcc 32 bits / PIC / static libgcc / static stdcxx / c++14 support"
    "Unix Makefiles"
)

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_common.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/compiler/gcc.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/cxx14.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/32bit.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/fpic.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/static-libgcc.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/static-libstdcxx.cmake")
