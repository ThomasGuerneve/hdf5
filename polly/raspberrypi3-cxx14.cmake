# Copyright (c) 2016, Brieuc Roblin
# All rights reserved.

if(DEFINED POLLY_RASPBERRYPI3_CMAKE)
  return()
else()
  set(POLLY_RASPBERRYPI3_CMAKE 1)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_init.cmake")

polly_init(
    "RaspberryPi 3 / 64 bits / C++14 / Cross Compile"
    "Unix Makefiles"
)

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_common.cmake")
include(polly_clear_environment_variables)
include(polly_fatal_error)
include(polly_add_cache_flag)

set(_rpi_error_msg) #if empty, then no errors!
string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PATH}"
    ""
    _is_empty
)
if(_is_empty)
  set(_rpi_error_msg
      "${_rpi_error_msg}\nAARCH64_CROSS_COMPILE_TOOLCHAIN_PATH environment variable not set. Set it to the absolute path of the \"bin\" directory for the toolchain"
  )
endif()

string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX}"
    ""
    _is_empty
)
if(_is_empty)
  set(_rpi_error_msg
      "${_rpi_error_msg}\nAARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX environment variable not set. Set it to the triplet of the toolchain (ex: aarch64-linux-gnu)"
  )
endif()

string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_SYSROOT}"
    ""
    _is_empty
)
if(_is_empty)
  set(_rpi_error_msg
      "${_rpi_error_msg}\nAARCH64_CROSS_COMPILE_SYSROOT environment variable not set. Set it to the sysroot to be used"
  )
endif()

string(COMPARE NOTEQUAL
    "${_rpi_error_msg}"
    ""
    _has_errors
)
if(_has_errors)
  polly_fatal_error(
      "RaspberyPi Toolchain configuration failed:"
      ${_rpi_error_msg}
  )
endif()

set(RASPBERRYPI3_COMPILE_FLAGS
    "-mcpu=cortex-a53 -mtune=cortex-a53"
    CACHE STRING "RaspberryPi 3 64bits Compile Flags"
)

list(APPEND HUNTER_TOOLCHAIN_UNDETECTABLE_ID "${RASPBERRYPI3_COMPILE_FLAGS}")

# We shouldn't try to hardcore the path, since the cross compiler for the Mac
# needs to be in a "case-sensitive" file system
set(CROSS_COMPILE_TOOLCHAIN_PATH
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PATH}"
    CACHE PATH "AARCH64 Toolchain Path"
)

# The prefix is what is known as the cross compiler triplet or quadruplet.
# ex: aarch64-linux-gnu (note: no dash at the end)
set(CROSS_COMPILE_TOOLCHAIN_PREFIX
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX}"
    CACHE STRING "AARCH64 Toolchain Prefix"
)

# The sysroot for the cross compile
set(CROSS_COMPILE_SYSROOT
    "$ENV{AARCH64_CROSS_COMPILE_SYSROOT}"
    CACHE PATH "AARCH64 sysroot"
)

# Set the crosscompiler
set(CMAKE_CROSSCOMPILING true)
set(CMAKE_CROSSCOMPILING_EMULATOR "qemu-aarch64-static")

include("${CMAKE_CURRENT_LIST_DIR}/compiler/gcc-cross-compile.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/flags/cxx14.cmake")

polly_add_cache_flag(CMAKE_C_FLAGS ${RASPBERRYPI3_COMPILE_FLAGS})
polly_add_cache_flag(CMAKE_CXX_FLAGS ${RASPBERRYPI3_COMPILE_FLAGS})
polly_add_cache_flag(CMAKE_SYSTEM_NAME "Linux")
polly_add_cache_flag(CMAKE_SYSTEM_PROCESSOR "armv8-a")
