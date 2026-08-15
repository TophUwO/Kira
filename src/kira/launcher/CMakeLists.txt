# ****************************************************************************************************************
# * Kira - cross-platform component-based modular application development framework written in C11               *
# *                                                                                                              *
# * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
# *                                                                                                              *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
# * this project. If this file is not present, visit                                                             *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
# ****************************************************************************************************************

 
list(APPEND KI_SOURCES "src/main.c")

# Set the target.
# The core module is the only module that is statically linked into the runtime because it contains the code that
# handles dynamic library loading.
add_executable(kira ${KI_SOURCES})

set_target_properties(kira PROPERTIES OUTPUT_NAME "kira")
target_link_libraries(kira PRIVATE kikrnl)
