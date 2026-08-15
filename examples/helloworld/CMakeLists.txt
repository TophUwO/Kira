# ****************************************************************************************************************
# * Kira - cross-platform component-based modular application development framework written in C11               *
# *                                                                                                              *
# * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
# *                                                                                                              *
# * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
# * this project. If this file is not present, visit                                                             *
# *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
# ****************************************************************************************************************


# (1) Collect the sources for the example.
set(SOURCES
    helloworld.c
)

# (2) Define a the module target and the executable launcher.
add_library(helloworldModule MODULE ${SOURCES})
add_executable(helloworld
    ${CMAKE_SOURCE_DIR}/src/kira/launcher/src/main.c
)

# (3) Set some target properties.
target_include_directories(helloworldModule PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_include_directories(helloworld PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(helloworldModule PRIVATE kikrnl)
target_link_libraries(helloworld PRIVATE kikrnl)
