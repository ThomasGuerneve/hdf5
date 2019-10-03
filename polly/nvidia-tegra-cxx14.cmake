# Copyright (c) 2016, Brieuc Roblin
# All rights reserved.

if(DEFINED POLLY_NVIDIA_JETSON_TK1_CMAKE)
  return()
else()
  set(POLLY_NVIDIA_JETSON_TK1_CMAKE 1)
endif()

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_init.cmake")

polly_init(
    "Nvidia Tegra / 64 bits/ big.LITTLE / C++14 / Cross Compile"
    "Unix Makefiles"
)

include("${CMAKE_CURRENT_LIST_DIR}/utilities/polly_common.cmake")
include(polly_clear_environment_variables)
include(polly_fatal_error)
include(polly_add_cache_flag)

set(_tegra_error_msg) #if empty, then no errors!
string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PATH}"
    ""
    _is_empty
)
if(_is_empty)
  set(_tegra_error_msg
      "${_tegra_error_msg}\nAARCH64_CROSS_COMPILE_TOOLCHAIN_PATH environment variable not set. Set it to the absolute path of the \"bin\" directory for the toolchain"
  )
endif()

string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX}"
    ""
    _is_empty
)
if(_is_empty)
  set(_tegra_error_msg
      "${_tegra_error_msg}\nAARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX environment variable not set. Set it to the triplet of the toolchain (ex: arm-linux-gnueabihf)"
  )
endif()

string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_SYSROOT}"
    ""
    _is_empty
)
if(_is_empty)
  set(_tegra_error_msg
      "${_tegra_error_msg}\nAARCH64_CROSS_COMPILE_SYSROOT environment variable not set. Set it to the sysroot to be used"
  )
endif()

string(COMPARE NOTEQUAL
    "${_tegra_error_msg}"
    ""
    _has_errors
)
if(_has_errors)
  polly_fatal_error(
      "Nvidia Tegra Toolchain configuration failed:"
      ${_tegra_error_msg}
  )
endif()

set(TEGRA_COMPILE_FLAGS
    "-march=armv8-a -mtune=cortex-a57.cortex-a53 -mcpu=cortex-a57.cortex-a53"
    CACHE STRING "Nvidia Tegra Compile Flags"
)

# We shouldn't try to hardcore the path, since the cross compiler for the Mac
# needs to be in a "case-sensitive" file system
set(CROSS_COMPILE_TOOLCHAIN_PATH
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PATH}"
    CACHE PATH "Nvidia Tegra Toolchain Path"
)

# The prefix is what is known as the cross compiler triplet or quadruplet.
# ex: arm-unknown-linux-gnueabihf (note: no dash at the end)
set(CROSS_COMPILE_TOOLCHAIN_PREFIX
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX}"
    CACHE STRING "Nvidia Tegra Toolchain Prefix"
)

# The sysroot for the cross compile
set(CROSS_COMPILE_SYSROOT
    "$ENV{AARCH64_CROSS_COMPILE_SYSROOT}"
    CACHE PATH "Nvidia Tegra sysroot"
)

# Set the crosscompiler
set(CMAKE_CROSSCOMPILING true)
set(CMAKE_CROSSCOMPILING_EMULATOR "qemu-aarch64-static")

include("${CMAKE_CURRENT_LIST_DIR}/flags/cxx14.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/compiler/gcc-cross-compile.cmake")

polly_add_cache_flag(CMAKE_C_FLAGS ${TEGRA_COMPILE_FLAGS})
polly_add_cache_flag(CMAKE_CXX_FLAGS ${TEGRA_COMPILE_FLAGS})
polly_add_cache_flag(CMAKE_SYSTEM_NAME "Linux")
polly_add_cache_flag(CMAKE_SYSTEM_PROCESSOR "armv8-a")
