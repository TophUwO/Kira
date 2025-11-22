# *****************************************************************************************************************
# * Mira - cross-platform 2-D role-playing game (RPG) game engine for desktop and mobile, and console platforms *
# *                                                                                                               *
# * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                   *
# *                                                                                                               *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of  *
# * this project. If this file is not present, visit                                                              *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                               *
# *****************************************************************************************************************

# Enumerate all common sources. 
file(
    GLOB_RECURSE KI_COMMON_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CMAKE_CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
)
set(KI_SOURCES ${KI_COMMON_SOURCES})

# Set the target.
# The core module is the only module that is statically linked into the runtime because it contains the code that
# handles dynamic library loading.
add_library(kicommon SHARED ${KI_SOURCES})
target_compile_definitions(kicommon PRIVATE KI_KERNEL_EXPORT)


