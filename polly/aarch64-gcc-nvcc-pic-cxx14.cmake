# Copyright (c) 2016, Brieuc Roblin
# All rights reserved.

if(DEFINED POLLY_AARCH64_GCC_NVCC_PIC_STATIC_GCC_STATIC_STDLIB_CXX14_CMAKE)
  return()
else()
  set(POLLY_AARCH64_GCC_NVCC_PIC_STATIC_GCC_STATIC_STDLIB_CXX14_CMAKE 1)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_init.cmake")

polly_init(
    "aarch64 Cross Compile / gcc / nvcc / PIC / C++14"
    "Unix Makefiles"
)

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_common.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/compiler/gcc-cross-compile-aarch64.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/compiler/nvcc.cmake")

include("${CMAKE_CURRENT_LIST_DIR}/flags/fpic.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/cxx14.cmake")

include(polly_nvcc_finalise)
