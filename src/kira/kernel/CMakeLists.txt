# *****************************************************************************************************************
# * Mira - cross-platform 2-D role-playing game (RPG) game engine for desktop and mobile, and console platforms *
# *                                                                                                               *
# * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                   *
# *                                                                                                               *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of  *
# * this project. If this file is not present, visit                                                              *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                               *
# *****************************************************************************************************************


# If we are on Windows and building a DLL, we need to set CMAKE_RUNTIME_OUTPUT_DIRECTORY to our module directory because
# Windows uses the same variable for both exe and dll files.
if (WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

# Enumerate all common sources. 
file(
    GLOB_RECURSE KI_COMMON_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)
set(KI_SOURCES ${KI_COMMON_SOURCES})
# Enumerate all external sources.
file(
    GLOB_RECURSE externAL_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ext/*.c
)
list(APPEND KI_SOURCES ${externAL_SOURCES})
# Enumerate all platform-specific sources.
file(
    GLOB_RECURSE KI_PLATFORM_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/${KI_PLATFORM_PREFIX}/*.c
)
list(APPEND KI_SOURCES ${KI_PLATFORM_SOURCES})

# Set the target.
# The core module is the only module that is statically linked into the runtime because it contains the code that
# handles dynamic library loading.
add_library(skkrnl SHARED ${KI_SOURCES})
set_target_properties(skkrnl PROPERTIES OUTPUT_NAME "kikrnl")
# Set target specific defines.
target_compile_definitions(skkrnl PRIVATE KI_KERNEL_EXPORT KI_KERNEL)


