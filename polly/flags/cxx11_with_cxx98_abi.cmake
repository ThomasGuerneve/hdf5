# Copyright (c) 2015, Ruslan Baratov
# All rights reserved.

if(DEFINED POLLY_FLAGS_CXX11_WITH_CXX98_ABI_CMAKE)
  return()
else()
  set(POLLY_FLAGS_CXX11_WITH_CXX98_ABI_CMAKE 1)
endif()

include(polly_add_cache_flag)

polly_add_cache_flag(CMAKE_CXX_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=0")

polly_add_cache_flag(CMAKE_NVCC_FLAGS "-D_GLIBCXX_USE_CXX11_ABI=0")

# There is no macro to detect this flags on toolchain calculation so we must
# mark this toolchain explicitly.
list(APPEND HUNTER_TOOLCHAIN_UNDETECTABLE_ID "cxx11_with_cxx98_abi")
