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
 * \file  linrand.c
 * \brief implements the Linux-specific cryptographically-secure random-number generator
 */


#if (defined KI_PLATFORM_LINUX)
/* stdlib includes */
#include <errno.h>

/* Linux includes */
#include <sys/random.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


KiEErrorCode KI_CALL KiPlatform_GetRandomBytes(KiTSize sizeInBytes, KiTVoid *resPtr) {
    KI_ASSERT(sizeInBytes > 0,   KiErr_SizeParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutParameter);

    /*
     * It is possible that getrandom() cannot fill the entire buffer at once. Thus, we repeat the process until the
     * buffer is full.
     */
    for (KiTSize rem = sizeInBytes, written = 0; rem > 0;) {
        /* (1) Fill buffer. */
        ssize_t const res = getrandom((void *)((char unsigned *)resPtr + written), (size_t)rem, 0);
        if (res == -1)
            return KiNativeErrorCodeToKiraErrorCode(errno);

        /* (2) Update state. */
        rem     -= (KiTSize)res;
        written += (KiTSize)res;
    }

    return KiErr_Ok;
}
#endif /* (defined KI_PLATFORM_LINUX) */
