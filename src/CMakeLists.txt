# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.19.2)

#
# Read our version
#
file(STRINGS "../common.sh" XA_UTILS_PRODUCT_VERSION REGEX "^XA_UTILS_VERSION=\"(.*)\"")
string(REGEX REPLACE "^XA_UTILS_VERSION=\"([0-9.]+)\"" "\\1" XA_UTILS_VERSION "${XA_UTILS_PRODUCT_VERSION}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE BOOL "" FORCE)

project(
  xa-utilities
  VERSION ${XA_UTILS_VERSION}
  DESCRIPTION "Xamarin.Android toolchain utilities"
  HOMEPAGE_URL "https://github.com/xamarin/xamarin-android-binutils"
  LANGUAGES C CXX
  )

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(COMPILER_DIAG_COLOR "Show compiler diagnostics/errors in color" ON)

if(NOT DEFINED BINUTILS_VERSION)
  message(FATAL_ERROR "Please set the BINUTILS_VERSION variable on command line (-DBINUTILS_VERSION=VERSION)")
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(IS_DEBUG True)
else()
  set(IS_DEBUG False)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)
include(CheckLinkerFlag)

macro(_compiler_has_flag _lang _flag)
  string(REGEX REPLACE "-|,|=" "_" _flag_name ${_flag})
  string(TOUPPER "${_lang}" _lang_upper)

  cmake_language(CALL check_${_lang}_compiler_flag "${_flag}" HAS_${_flag_name}_${_lang_upper})
  if(HAS_${_flag_name}_${_lang_upper})
    set(COMPILER_FLAG_FOUND True)
  else()
    set(COMPILER_FLAG_FOUND False)
  endif()
endmacro()

macro(cxx_compiler_has_flag _flag)
  _compiler_has_flag(cxx ${_flag})
endmacro()

macro(c_compiler_has_flag _flag)
  _compiler_has_flag(c ${_flag})
endmacro()

macro(_linker_has_flag _lang _flag)
  string(REGEX REPLACE "-|,|=" "_" _flag_name ${_flag})
  string(TOUPPER "${_lang}" _lang_upper)

  check_linker_flag(${_lang} "${_flag}" HAS_${_flag_name}_LINKER_${_lang_upper})
  if(HAS_${_flag_name}_LINKER_${_lang_upper})
    set(LINKER_FLAG_FOUND True)
  else()
    set(LINKER_FLAG_FOUND False)
  endif()
endmacro()

macro(cxx_linker_has_flag _flag)
  _linker_has_flag(CXX ${_flag})
endmacro()

macro(c_linker_has_flag _flag)
  _linker_has_flag(C ${_flag})
endmacro()

set(COMMON_COMPILER_ARGS
  -fstack-protector-strong
  -fstrict-return
  -fno-strict-aliasing
  -ffunction-sections
  -funswitch-loops
  -finline-limit=500
  -Wa,-noexecstack
  -fpic
  -g
  -Wa,--noexecstack
  -Wformat
  -Werror=format-security
  -Wall
  -Wextra
  -Wnull-dereference
  -Wmisleading-indentation
  -Wunused
  -Wpedantic
  -Wsuggest-final-types
  -Wsuggest-final-methods
  -Wint-to-pointer-cast
  -Wunguarded-availability
  -Wsuggest-override
  )

set(COMMON_LINKER_ARGS
  -fstack-protector-strong
  -static-libstdc++
  -static-libgcc
  LINKER:-fstrict-return
  LINKER:-z,now
  LINKER:-z,relro
  LINKER:-z,noexecstack
  )

if(COMPILER_DIAG_COLOR)
  list(APPEND COMMON_COMPILER_ARGS -fdiagnostics-color=always)
else()
  list(APPEND COMMON_COMPILER_ARGS -fdiagnostics-color=auto)
endif()

if(IS_DEBUG)
  add_compile_definitions(DEBUG)
  list(APPEND COMMON_COMPILER_ARGS
    -ggdb3
    -fno-omit-frame-pointer
    -O0
    )
else()
  add_compile_definitions(NDEBUG)
  list(APPEND COMMON_COMPILER_ARGS -fomit-frame-pointer -O2)
endif()

foreach(flag ${COMMON_COMPILER_ARGS})
  cxx_compiler_has_flag(${flag})
  if(COMPILER_FLAG_FOUND)
    list(APPEND XA_UTILS_CXX_FLAGS "${flag}")
  endif()
endforeach()

foreach(flag ${COMMON_COMPILER_ARGS})
  c_compiler_has_flag(${flag})
  if(COMPILER_FLAG_FOUND)
    list(APPEND XA_UTILS_C_FLAGS "${flag}")
  endif()
endforeach()

foreach(flag ${COMMON_LINKER_ARGS})
  cxx_linker_has_flag(${flag})
  if(LINKER_FLAG_FOUND)
    list(APPEND XA_UTILS_CXX_LINKER_FLAGS "${flag}")
  endif()
endforeach()

foreach(flag ${COMMON_LINKER_ARGS})
  c_linker_has_flag(${flag})
  if(LINKER_FLAG_FOUND)
    list(APPEND XA_UTILS_C_LINKER_FLAGS "${flag}")
  endif()
endforeach()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
  add_compile_definitions(_GNU_SOURCE)
endif()

add_compile_definitions(XA_UTILS_VERSION="${PROJECT_VERSION}")
add_compile_definitions(BINUTILS_VERSION="${BINUTILS_VERSION}")
add_compile_definitions(LLVM_VERSION="${LLVM_VERSION}")

add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${XA_UTILS_CXX_FLAGS}>")
add_compile_options("$<$<COMPILE_LANGUAGE:C>:${XA_UTILS_C_FLAGS}>")

add_link_options("$<$<COMPILE_LANGUAGE:CXX>:${XA_UTILS_CXX_LINKER_FLAGS}>")
add_link_options("$<$<COMPILE_LANGUAGE:C>:${XA_UTILS_C_LINKER_FLAGS}>")

if(WIN32)
  include_directories(compat)
endif()

add_subdirectory(gas)
