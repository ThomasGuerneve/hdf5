# Copyright (c) 2016-2017, Ruslan Baratov
# All rights reserved.

if(DEFINED POLLY_GCC_7_NVCC_PIC_STATIC_STDLIB_CMAKE_)
  return()
else()
  set(POLLY_GCC_7_NVCC_PIC_STATIC_STDLIB_CMAKE_ 1)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_init.cmake")

polly_init(
    "gcc 7 / nvcc / PIC / static stdlib / c++14 support"
    "Unix Makefiles"
)

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_common.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/compiler/gcc-7.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/compiler/nvcc.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/flags/fpic.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/static-std.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/cxx14.cmake")

include(polly_nvcc_finalise)
