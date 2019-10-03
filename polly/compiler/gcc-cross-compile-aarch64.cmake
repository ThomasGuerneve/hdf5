
if(DEFINED POLLY_COMPILER_GCC_CROSS_AARCH64_CMAKE_)
  return()
else()
  set(POLLY_COMPILER_GCC_CROSS_AARCH64_CMAKE_ 1)
endif()

include(polly_clear_environment_variables)

set(_armv8_error_msg) #if empty, then no errors!
string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PATH}"
    ""
    _is_empty
)
if(_is_empty)
  set(_armv8_error_msg
      "${_armv8_error_msg}\nAARCH64_CROSS_COMPILE_TOOLCHAIN_PATH environment variable not set. Set it to the absolute path of the \"bin\" directory for the toolchain"
  )
endif()

string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX}"
    ""
    _is_empty
)
if(_is_empty)
  set(_armv8_error_msg
      "${_armv8_error_msg}\nAARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX environment variable not set. Set it to the triplet of the toolchain (ex: aarch64-linux-gnu)"
  )
endif()

string(COMPARE EQUAL
    "$ENV{AARCH64_CROSS_COMPILE_SYSROOT}"
    ""
    _is_empty
)
if(_is_empty)
  set(_armv8_error_msg
      "${_armv8_error_msg}\nAARCH64_CROSS_COMPILE_SYSROOT environment variable not set. Set it to the sysroot to be used"
  )
endif()

string(COMPARE NOTEQUAL
    "${_armv8_error_msg}"
    ""
    _has_errors
)
if(_has_errors)
  polly_fatal_error(
      "aarch64 Toolchain configuration failed:"
      ${_armv8_error_msg}
  )
endif()

set(AARCH64_COMPILE_FLAGS
    "-march=armv8-a"
    CACHE STRING "aarch64 Compile Flags"
)

list(APPEND HUNTER_TOOLCHAIN_UNDETECTABLE_ID "${AARCH64_COMPILE_FLAGS}")

# We shouldn't try to hardcore the path, since the cross compiler for the Mac
# needs to be in a "case-sensitive" file system
set(CROSS_COMPILE_TOOLCHAIN_PATH
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PATH}"
    CACHE PATH "aarch64 Toolchain Path"
)

# The prefix is what is known as the cross compiler triplet or quadruplet.
# ex: aarch64-linux-gnu (note: no dash at the end)
set(CROSS_COMPILE_TOOLCHAIN_PREFIX
    "$ENV{AARCH64_CROSS_COMPILE_TOOLCHAIN_PREFIX}"
    CACHE STRING "aarch64 Toolchain Prefix"
)

# The sysroot for the cross compile
set(CROSS_COMPILE_SYSROOT
    "$ENV{AARCH64_CROSS_COMPILE_SYSROOT}"
    CACHE PATH "AARCH64 sysroot"
)

# Set the crosscompiler
set(CMAKE_CROSSCOMPILING true)
set(CMAKE_CROSSCOMPILING_EMULATOR "qemu-aarch64-static")

include("${CMAKE_CURRENT_LIST_DIR}/gcc-cross-compile.cmake")

polly_add_cache_flag(CMAKE_C_FLAGS ${AARCH64_COMPILE_FLAGS})
polly_add_cache_flag(CMAKE_CXX_FLAGS ${AARCH64_COMPILE_FLAGS})
polly_add_cache_flag(NVCC_XCOMPILE_FLAGS ${AARCH64_COMPILE_FLAGS})

polly_add_cache_flag(CMAKE_SYSTEM_NAME "Linux")
polly_add_cache_flag(CMAKE_SYSTEM_PROCESSOR "aarch64")

polly_add_cache_flag(CMAKE_FIND_ROOT_PATH ${CROSS_COMPILE_SYSROOT})
polly_add_cache_flag(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY "ONLY")
polly_add_cache_flag(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE "ONLY")
