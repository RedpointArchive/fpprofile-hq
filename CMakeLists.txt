﻿# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.7)

project ("fpprofile")

include(TargetArch.cmake)
target_architecture(TARGET_CPU_ARCH)

message("Detected target architecture: ${TARGET_CPU_ARCH}")

if(${TARGET_CPU_ARCH} STREQUAL "i386")
    if(MSVC)
        add_compile_options(/arch:SSE2)
    elseif(UNIX)
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
            add_compile_options(-mno-80387 -march=nocona -mfpmath=sse -msse2 -frounding-math -fsignaling-nans -mieee-fp -mno-fp-ret-in-387)
        elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            add_compile_options(-march=nocona -msse2 -mno-x87)
        endif()
    endif()
elseif(${TARGET_CPU_ARCH} STREQUAL "x86_64")
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options(-mno-80387 -march=nocona -mfpmath=sse -msse2 -frounding-math -fsignaling-nans -mieee-fp -mno-fp-ret-in-387)
    elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        add_compile_options(-march=nocona -msse2 -mno-x87)
    endif()
elseif(${TARGET_CPU_ARCH} STREQUAL "ia64")
    message(FATAL_ERROR "IA64 architecture is not supported.")
elseif(${TARGET_CPU_ARCH} STREQUAL "ppc")
    message(FATAL_ERROR "PPC architecture is not supported.")
elseif(${TARGET_CPU_ARCH} STREQUAL "ppc64")
    message(FATAL_ERROR "PPC64 architecture is not supported.")
elseif(${TARGET_CPU_ARCH} STREQUAL "arm")
elseif(${TARGET_CPU_ARCH} STREQUAL "armv5")
elseif(${TARGET_CPU_ARCH} STREQUAL "armv6")
elseif(${TARGET_CPU_ARCH} STREQUAL "armv7")
elseif(${TARGET_CPU_ARCH} STREQUAL "unknown")
    message(FATAL_ERROR "Target architecture could not be detected.")
endif()

add_subdirectory ("sodium")
add_subdirectory ("fpprofile")
