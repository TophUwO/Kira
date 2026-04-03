/*****************************************************************************************************************
 * Kira - cross-platform 2-D role-playing game (RPG) game engine for desktop and mobile, and console platforms *
 *                                                                                                               *
 * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                   *
 *                                                                                                               *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of  *
 * this project. If this file is not present, visit                                                              *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                               *
 *****************************************************************************************************************/

/**
 * \file  windir.c
 * \brief implements Windows(R)-specific routines regarding directory handling
*/
#if (defined KI_PLATFORM_LINUX)


/* Kira includes */
#include <kira/kernel/int/platform.h>



KiTChar KI_CALL KiPlatform_GetPathSeparator(KiTVoid) {
    return '/';
}


#endif /* KI_PLATFORM_LINUX */


