# Copyright (c) 2016, Brieuc Roblin
# All rights reserved.

if(DEFINED POLLY_ARMHF_CMAKE)
  return()
else()
  set(POLLY_ARMHF_CMAKE 1)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_init.cmake")

polly_init(
    "armv7-a Cross Compile / armhf / C++14"
    "Unix Makefiles"
)

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_common.cmake")
include(polly_clear_environment_variables)
include(polly_fatal_error)
include(polly_add_cache_flag)

set(_armv7_error_msg) #if empty, then no errors!
string(COMPARE EQUAL
    "$ENV{ARMHF_CROSS_COMPILE_TOOLCHAIN_PATH}"
    ""
    _is_empty
)
if(_is_empty)
  set(_armv7_error_msg
      "${_armv7_error_msg}\nARMHF_CROSS_COMPILE_TOOLCHAIN_PATH environment variable not set. Set it to the absolute path of the \"bin\" directory for the toolchain"
  )
endif()

string(COMPARE EQUAL
    "$ENV{ARMHF_CROSS_COMPILE_TOOLCHAIN_PREFIX}"
    ""
    _is_empty
)
if(_is_empty)
  set(_armv7_error_msg
      "${_armv7_error_msg}\nARMHF_CROSS_COMPILE_TOOLCHAIN_PREFIX environment variable not set. Set it to the triplet of the toolchain (ex: aarch64-linux-gnu)"
  )
endif()

string(COMPARE EQUAL
    "$ENV{ARMHF_CROSS_COMPILE_SYSROOT}"
    ""
    _is_empty
)
if(_is_empty)
  set(_armv7_error_msg
      "${_armv7_error_msg}\nARMHF_CROSS_COMPILE_SYSROOT environment variable not set. Set it to the sysroot to be used"
  )
endif()

string(COMPARE NOTEQUAL
    "${_armv7_error_msg}"
    ""
    _has_errors
)
if(_has_errors)
  polly_fatal_error(
      "armhf Toolchain configuration failed:"
      ${_armv7_error_msg}
  )
endif()

set(ARMHF_COMPILE_FLAGS
    "-march=armv7-a"
    CACHE STRING "armhf Compile Flags"
)

list(APPEND HUNTER_TOOLCHAIN_UNDETECTABLE_ID "${ARMHF_COMPILE_FLAGS}")

# We shouldn't try to hardcore the path, since the cross compiler for the Mac
# needs to be in a "case-sensitive" file system
set(CROSS_COMPILE_TOOLCHAIN_PATH
    "$ENV{ARMHF_CROSS_COMPILE_TOOLCHAIN_PATH}"
    CACHE PATH "armhf Toolchain Path"
)

# The prefix is what is known as the cross compiler triplet or quadruplet.
# ex: aarch64-linux-gnu (note: no dash at the end)
set(CROSS_COMPILE_TOOLCHAIN_PREFIX
    "$ENV{ARMHF_CROSS_COMPILE_TOOLCHAIN_PREFIX}"
    CACHE STRING "armf Toolchain Prefix"
)

# The sysroot for the cross compile
set(CROSS_COMPILE_SYSROOT
    "$ENV{ARMHF_CROSS_COMPILE_SYSROOT}"
    CACHE PATH "armhf sysroot"
)

# Set the crosscompiler
set(CMAKE_CROSSCOMPILING true)
set(CMAKE_CROSSCOMPILING_EMULATOR "qemu-arm-static")

include("${CMAKE_CURRENT_LIST_DIR}/compiler/gcc-cross-compile.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/cxx14.cmake")

polly_add_cache_flag(CMAKE_C_FLAGS ${ARMHF_COMPILE_FLAGS})
polly_add_cache_flag(CMAKE_CXX_FLAGS ${ARMHF_COMPILE_FLAGS})
polly_add_cache_flag(CMAKE_SYSTEM_NAME "Linux")
polly_add_cache_flag(CMAKE_SYSTEM_PROCESSOR "armv7-a")
