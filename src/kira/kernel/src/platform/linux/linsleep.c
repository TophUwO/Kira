/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2026 TophUwO <tophuwo01@gmail.com>                                                                  *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

/**
 * \file  linsleep.c
 * \brief implements Linux-specific routine for putting a thread to sleep
*/
#if (defined KI_PLATFORM_LINUX)


/* stdlib includes */
#include <time.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


KiTVoid KI_CALL KiPlatform_Sleep(KiTUint32 sleepMs) {
    long res;
    {
        KiTBool const isOverflow = __builtin_mul_overflow((KiTUint32)sleepMs, (KiTUint32)1000000, &res);

        if (isOverflow == KI_TRUE)
            return;
    }

    nanosleep(&(struct timespec const){ .tv_sec = 0, .tv_nsec = res }, nullptr);
}


#endif /* KI_PLATFORM_LINUX */


