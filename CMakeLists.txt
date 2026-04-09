# ****************************************************************************************************************
# * Kira - cross-platform component-based modular application development framework written in C11               *
# *                                                                                                              *
# * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                  *
# *                                                                                                              *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
# * this project. If this file is not present, visit                                                             *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
# ****************************************************************************************************************


# Define project.
cmake_minimum_required(VERSION 3.30 FATAL_ERROR)
project(Kira
    VERSION      0.1.0.1
    DESCRIPTION  "cross-platform component-based modular application development framework written in C11"
    HOMEPAGE_URL "https://www.github.com/TophUwO/Kira.git"
    LANGUAGES    C
)

# Set basic properties.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED true)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/modules")

# Detect platform.
set(CMAKE_KIRA_PNAMES_BSD     "DragonFly" "FreeBSD" "NetBSD" "OpenBSD")
set(CMAKE_KIRA_PNAMES_DARWIN  "Darwin")
set(CMAKE_KIRA_PNAMES_LINUX   "Linux")
set(CMAKE_KIRA_PNAMES_WINDOWS "Windows" "WindowsCE" "MSYS" "CYGWIN")

if (CMAKE_SYSTEM_NAME IN_LIST CMAKE_KIRA_PNAMES_BSD)
    set(KIRA_TARGET_CAT "BSD")

    add_compile_definitions(KI_PLATFORM_BSD)
elseif (CMAKE_SYSTEM_NAME IN_LIST CMAKE_KIRA_PNAMES_DARWIN)
    set(KIRA_TARGET_CAT "Darwin")

    add_compile_definitions(KI_PLATFORM_DARWIN)
elseif (CMAKE_SYSTEM_NAME IN_LIST CMAKE_KIRA_PNAMES_LINUX)
    set(KIRA_TARGET_CAT "Linux")

    add_compile_definitions(KI_PLATFORM_LINUX)
elseif (CMAKE_SYSTEM_NAME IN_LIST CMAKE_KIRA_PNAMES_WINDOWS)
    set(KIRA_TARGET_CAT "Windows")

    add_compile_definitions(KI_PLATFORM_WINDOWS)
else()
    set(KIRA_TARGET_CAT "Unknown")

    message(FATAL_ERROR "kira: Unsupported platform ${CMAKE_SYSTEM_NAME}.")
endif()
set(KIRA_TARGET_NAME      ${CMAKE_SYSTEM_NAME})
set(KIRA_TARGET_NAME_FULL ${CMAKE_SYSTEM})

string(TOLOWER "${KIRA_TARGET_CAT}" CMAKE_KIRA_PSRCDIR)

# Check target architecture.
set(CMAKE_KIRA_SUPPORTED_BITNESSES 32 64)
math(EXPR KIRA_TARGET_BITNESS "${CMAKE_SIZEOF_VOID_P} * 8")

if (NOT KIRA_TARGET_BITNESS IN_LIST CMAKE_KIRA_SUPPORTED_BITNESSES)
    message(FATAL_ERROR "kira: Kira only supports 32- and 64-bit platforms at the moment. Current architecture: ${KIRA_TARGET_BITNESS}-bit.")
endif()

# Detect compiler.
set(CMAKE_KIRA_SUPPORTED_COMPILERS "AppleClang" "Clang" "GNU" "Intel" "MSVC")

if (CMAKE_C_COMPILER_ID IN_LIST CMAKE_KIRA_SUPPORTED_COMPILERS)
    message(STATUS "kira: Detected supported compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}.")

    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        message(STATUS "kira: Compiling for platform ${CMAKE_SYSTEM_PROCESSOR}.")
    else()
        message(STATUS "kira: Compiling for platform ${CMAKE_C_COMPILER_ARCHITECTURE_ID}.")
    endif()
else()
    message(FATAL_ERROR "kira: Kira does not support ${CMAKE_C_COMPILER_ID} at the moment. Choose one of the following: ${CMAKE_KIRA_SUPPORTED_COMPILERS}.")
endif()

if (CMAKE_C_COMPILER_ID STREQUAL "AppleClang")
    add_compile_definitions(KI_HOST_APLCLANG)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_definitions(KI_HOST_CLANG)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_definitions(KI_HOST_GCC)
elseif(CMAKE_C_COMPILER_ID STREQUAL "Intel")
    add_compile_definitions(KI_HOST_ICC)
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
    add_compile_definitions(KI_HOST_MSVC)
endif()

set(KIRA_COMPILER_CSTD   ${CMAKE_C_STANDARD})
set(KIRA_COMPILER_ID     ${CMAKE_C_COMPILER_ID})
set(KIRA_COMPILER_VER    ${CMAKE_C_COMPILER_VERSION})
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(KIRA_COMPILER_ARCH ${CMAKE_SYSTEM_PROCESSOR})
else()
    set(KIRA_COMPILER_ARCH ${CMAKE_C_COMPILER_ARCHITECTURE_ID})
endif()
set(KIRA_COMPILER_TARGET ${CMAKE_C_COMPILER_TARGET})

# Get build system info.
set(KIRA_BUILDSYS_ID    "CMake")
set(KIRA_BUILDSYS_VER   ${CMAKE_VERSION})
set(KIRA_BUILDSYS_GEN   ${CMAKE_GENERATOR})
set(KIRA_BUILDSYS_BTYPE ${CMAKE_BUILD_TYPE})

# Get configuration.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(KI_CONFIG_DEBUG)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(KI_CONFIG_RELEASE)
elseif (CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    add_compile_definitions(KI_CONFIG_RELEASEMINSIZE)
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    add_compile_definitions(KI_CONFIG_OPTIMIZEDDEBUG)
else()
    # Only support the default build types.
    message(
        FATAL_ERROR
        "Kira currently only supports the default build types, that is, "
        "\"Debug\", \"Release\", \"MinSizeRel\", \"RelWithDebInfo\". Type "
        "\"${CMAKE_BUILD_TYPE}\" is not supported."
    )
endif()

# Set global include directory.
include_directories(${CMAKE_SOURCE_DIR}/src)
add_compile_definitions(KI_TARGET_X64)
add_compile_options(-pedantic-errors -fno-strict-aliasing -m64)

# Add kernel target.
add_subdirectory(${CMAKE_SOURCE_DIR}/src/kira/kernel)
# Add launcher target.
add_subdirectory(${CMAKE_SOURCE_DIR}/src/kira/launcher)


