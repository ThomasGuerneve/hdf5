
if(DEFINED POLLY_UTILITIES_POLLY_NVCC_FINALISE_CMAKE)
  return()
else()
  set(POLLY_UTILITIES_POLLY_NVCC_FINALISE_CMAKE 1)
endif()

if(NOT (DEFINED POLLY_COMPILER_NVCC_CMAKE_))
  polly_fatal_error("polly_nvcc_finalise was called without including the nvcc toolchain file")
endif()

# If we have arguments that need passing with '-Xcompiler' then add them to the CMAKE_NVCC_FLAGS variable
if(NVCC_XCOMPILE_FLAGS)
  set(XCOMPILE_ARGUMENT "-Xcompiler \"${NVCC_XCOMPILE_FLAGS}\"")
  polly_add_cache_flag(CMAKE_NVCC_FLAGS "${XCOMPILE_ARGUMENT};")
endif()

# Set legacy CUDA flags variable
set(CUDA_NVCC_FLAGS "${CMAKE_NVCC_FLAGS}" CACHE STRING "" FORCE)

# Turn off host compiler flag propagation
set(CUDA_PROPAGATE_HOST_FLAGS "OFF" CACHE STRING "Propagate C/CXX_FLAGS and friends to the host compiler via -Xcompile" FORCE)
