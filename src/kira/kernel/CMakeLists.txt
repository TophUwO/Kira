# ****************************************************************************************************************
# * Kira - cross-platform component-based modular application development framework written in C11               *
# *                                                                                                              *
# * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
# *                                                                                                              *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
# * this project. If this file is not present, visit                                                             *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
# ****************************************************************************************************************


# Enumerate all common sources. 
file(
    GLOB KI_COMMON_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)
set(KI_SOURCES ${KI_COMMON_SOURCES})
# Enumerate all external sources.
file(
    GLOB_RECURSE KI_EXTERNAL_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ext/*.c
)
list(APPEND KI_SOURCES ${KI_EXTERNAL_SOURCES})
# Enumerate all platform-specific sources.
file(
    GLOB_RECURSE KI_PLATFORM_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/platform/${CMAKE_KIRA_PSRCDIR}/*.c
)
list(APPEND KI_SOURCES ${KI_PLATFORM_SOURCES})

# Set the target.
# The core module is the only module that is statically linked into the runtime because it contains the code that
# handles dynamic library loading.
add_library(kikrnl SHARED ${KI_SOURCES})
# Set target specific defines.
target_compile_definitions(kikrnl PRIVATE KI_KERNEL_EXPORT KI_KERNEL)
