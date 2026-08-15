/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

/**
 * \file  winsleep.c
 * \brief implements the native sleep() routine for the Windows(R) platform
 */


#if (defined KI_PLATFORM_WINDOWS)
/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


KiTVoid KI_CALL KiPlatform_Sleep(KiTUint32 sleepMs) {
    Sleep((DWORD)sleepMs);
}
#endif /* (defined KI_PLATFORM_WINDOWS) */
