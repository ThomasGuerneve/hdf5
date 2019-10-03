
if(DEFINED POLLY_COMPILER_NVCC_CMAKE_)
  return()
else()
  set(POLLY_COMPILER_NVCC_CMAKE_ 1)
endif()

# Check that nvcc is available
find_program(CMAKE_NVCC_COMPILER nvcc
  PATHS ENV CUDA_TOOLKIT_ROOT
  PATH_SUFFIXES bin
  NO_DEFAULT_PATH)

if(NOT CMAKE_NVCC_COMPILER)
  polly_fatal_error("nvcc not found in CUDA_TOOLKIT_ROOT/bin \"${CUDA_TOOLKIT_ROOT}/bin\"")
endif()

set(
    CMAKE_NVCC_COMPILER
    "${CMAKE_NVCC_COMPILER}"
    CACHE
    STRING
    "CUDA nvcc compiler"
    FORCE
)

# Check that a gcc compiler is available
string(REGEX MATCH "gcc" gcc_found "${CMAKE_C_COMPILER}")
if (NOT gcc_found)
  polly_fatal_error("nvcc requires gcc")
endif()

# Check nvcc and gcc versions are compatible
# Get GCC version - Note that we can't query CMAKE_C_COMPILER_VERSION because it isn't set yet
execute_process(
  COMMAND ${CMAKE_CURRENT_LIST_DIR}/../scripts/gcc-version.sh "${CMAKE_C_COMPILER}"
  OUTPUT_VARIABLE gcc_version
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE gcc_result
  ERROR_VARIABLE gcc_error
  ERROR_STRIP_TRAILING_WHITESPACE)

if(NOT gcc_version)
  polly_fatal_error(
"Could not determine gcc version. 
COMMAND : \"${CMAKE_CURRENT_LIST_DIR}/../scripts/gcc-version.sh  ${CMAKE_C_COMPILER}\"
RESULT : \"${gcc_result}\"
ERROR : \"${gcc_error}\"")
endif()

# Get nvcc version
execute_process(
  COMMAND ${CMAKE_CURRENT_LIST_DIR}/../scripts/nvcc-version.sh ${CMAKE_NVCC_COMPILER}
  OUTPUT_VARIABLE nvcc_version
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE nvcc_result
  ERROR_VARIABLE nvcc_error
  ERROR_STRIP_TRAILING_WHITESPACE)

if(NOT nvcc_version)
  polly_fatal_error(
"Could not determine nvcc version. 
COMMAND : \"${CMAKE_CURRENT_LIST_DIR}/../scripts/nvcc-version.sh ${CMAKE_NVCC_COMPILER}\"
RESULT : \"${nvcc_result}\"
ERROR : \"${nvcc_error}\"")
endif()

polly_status_debug("GCC : \"${gcc_version}\"   nvcc_version : \"${nvcc_version}\"")

set(error_message 
"GCC ${gcc_version} is not compatible with nvcc ${nvcc_version}.
For a list of compatible host compilers see:
https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html")

# gcc-7 requires CUDA 9.2+
if((${gcc_version} VERSION_GREATER_EQUAL "7.0") AND (${nvcc_version} VERSION_LESS "9.2"))
  polly_fatal_error(${error_message})
endif()

# gcc-6 requires CUDA 9.0+
if((${gcc_version} VERSION_GREATER_EQUAL "6.0") AND (${nvcc_version} VERSION_LESS "9.0"))
  polly_fatal_error(${error_message})
endif()

# gcc-5 requires CUDA 8.0+
if((${gcc_version} VERSION_GREATER_EQUAL "5.0") AND (${nvcc_version} VERSION_LESS "8.0"))
  polly_fatal_error(${error_message})
endif()
