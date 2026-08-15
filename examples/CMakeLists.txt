# ****************************************************************************************************************
# * Kira - cross-platform component-based modular application development framework written in C11               *
# *                                                                                                              *
# * (c) 2024-2025 Toffi <tophuwo01@gmail.com>                                                                    *
# *                                                                                                              *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
# * this project. If this file is not present, visit                                                             *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
# ****************************************************************************************************************


# (1) Get all immediate subdirectories.
file(GLOB children RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*")

# (2) Go over the found subdirectories and include them into the CMake project.
foreach (child ${children})
    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${child}")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/${child}")
    endif()
endforeach()
