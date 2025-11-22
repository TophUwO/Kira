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
 * \file  wincvt.c
 * \brief implements the to-native-string conversions for the Windows platform
 */
#if (defined KI_PLATFORM_WINDOWS)


/* stdlib includes */
#include <stdlib.h>

/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


KiTVoid *KI_CALL KiPlatform_CreateFromKiraEncoding(KiTChar const *srcPtr) {
    /* Allocate and convert message to UTF-16. */
    WCHAR *cvtRes;
    KiTSize const reqSize = (KiTSize)MultiByteToWideChar(CP_UTF8, 0, srcPtr, -1, nullptr, 0);
    if ((cvtRes = calloc(1, reqSize * sizeof *cvtRes)) == nullptr)
        return nullptr;

    /* Convert to UTF16. */
    int const res = MultiByteToWideChar(CP_UTF8, 0, srcPtr, -1, cvtRes, reqSize);
    if (res == 0) {
        free(cvtRes);

        return nullptr;
    }

    /* All good. */
    return cvtRes;
}

KiTChar *KI_CALL KiPlatform_CreateFromNativeEncoding(KiTVoid const *srcPtr) {
    /* Allocate and convert message to UTF-16. */
    KiTChar *cvtRes;
    KiTSize const reqSize = (KiTSize)WideCharToMultiByte(CP_UTF8, 0, srcPtr, -1, nullptr, 0, nullptr, nullptr);
    if ((cvtRes = calloc(1, reqSize * sizeof *cvtRes)) == nullptr)
        return nullptr;

    /* Convert to UTF16. */
    int const res = WideCharToMultiByte(CP_UTF8, 0, srcPtr, -1, cvtRes, reqSize, nullptr, nullptr);
    if (res == 0) {
        free(cvtRes);

        return nullptr;
    }

    /* All good. */
    return cvtRes;
}
#endif 


