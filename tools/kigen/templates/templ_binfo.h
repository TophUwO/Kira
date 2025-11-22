/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                  *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/


#pragma once


/* host platform information */
#define KIRA_INFO_PLATFORM_CAT       "@KIRA_PLATFORM_CAT@"
#define KIRA_INFO_PLATFORM_NAME      "@KIRA_PLATFORM_NAME@"
#define KIRA_INFO_PLATFORM_NAME_FULL "@KIRA_PLATFORM_NAME_FULL@"
#define KIRA_INFO_PLATFORM_BITNESS    @KIRA_PLATFORM_BITNESS@

/* compiler and target information */
#define KIRA_INFO_COMPILER_CSTD      "@KIRA_COMPILER_CSTD@"
#define KIRA_INFO_COMPILER_ID        "@INFO_COMPILER_ID@"
#define KIRA_INFO_COMPILER_VER       "@INFO_COMPILER_VER@"
#define KIRA_INFO_COMPILER_ARCH      "@INFO_COMPILER_ARCH@"
#define KIRA_INFO_COMPILER_TARGET    "@INFO_COMPILER_TARGET@"

/* information on the build system (if applicable) */
#define KIRA_INFO_BUILDSYS_ID        "@KIRA_BUILDSYS_ID@"
#define KIRA_INFO_BUILDSYS_VER       "@KIRA_BUILDSYS_VER@"
#define KIRA_INFO_BUILDSYS_GEN       "@KIRA_BUILDSYS_GEN@"
#define KIRA_INFO_BUILDSYS_BTYPE     "@KIRA_BUILDSYS_BTYPE@"

/* latest git status */
#define KIRA_INFO_GIT_BRANCH_NAME    "@KIRA_GIT_BRANCH_NAME@"
#define KIRA_INFO_GIT_CHASH_LONG     "@KIRA_GIT_CHASH_LONG@"
#define KIRA_INFO_GIT_CHASH_SHORT    "@KIRA_GIT_CHASH_SHORT@"
#define KIRA_INFO_GIT_IS_DIRTY        @KIRA_GIT_IS_DIRTY@


