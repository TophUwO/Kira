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
 * \file  wintransc.c
 * \brief implements the to-native-string conversions for the Windows platform
 */
#if (defined KI_PLATFORM_WINDOWS)


/* stdlib includes */
#include <stdlib.h>

/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


KiTVoid *KI_CALL KiPlatform_CreateFromKiraEncoding(KiTChar const *srcPtr, KiTInt64 maxLen, KiTSize *sizePtr, KiTSize *memSizePtr) {
    KI_ASSERT(srcPtr != nullptr,  KiErr_InParameter);
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);
    KI_ASSERT(memSizePtr != nullptr, KiErr_OutParameter);

    WCHAR *cvtRes;
    KiTSize reqSize;
    SetLastError(0);
    {
        reqSize = (KiTSize)MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, srcPtr, KI_MAX(maxLen, -1), nullptr, 0);
    }
    if (GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
        /* Encoding error. */
        *sizePtr = 0;

        return nullptr;
    }

    if ((cvtRes = calloc(1, reqSize * sizeof *cvtRes)) == nullptr) {
        *sizePtr = 0;

        return nullptr;
    }

    KiTSize const res = MultiByteToWideChar(CP_UTF8, 0, srcPtr, KI_MAX(maxLen, -1), cvtRes, reqSize);
    if (res == 0) {
        free(cvtRes);

        *sizePtr = 0;
        return nullptr;
    }

    *sizePtr    = res     * sizeof *cvtRes;
    *memSizePtr = reqSize * sizeof *cvtRes;
    return cvtRes;
}

KiTChar *KI_CALL KiPlatform_CreateFromNativeEncoding(KiTVoid const *srcPtr, KiTInt64 maxLen, KiTSize *sizePtr, KiTSize *memSizePtr) {
    KI_ASSERT(srcPtr != nullptr,  KiErr_InParameter);
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);
    KI_ASSERT(memSizePtr != nullptr, KiErr_OutParameter);

    KiTChar *cvtRes;
    KiTSize reqSize;
    SetLastError(0);
    {
        reqSize = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, srcPtr, KI_MAX(maxLen, -1), nullptr, 0, nullptr, nullptr);
    }
    if (GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
        /* Encoding error. */
        *sizePtr = 0;

        return nullptr;
    }

    if ((cvtRes = calloc(1, reqSize * sizeof *cvtRes)) == nullptr) {
        *sizePtr = 0;

        return nullptr;
    }

    KiTSize const res = WideCharToMultiByte(CP_UTF8, 0, srcPtr, KI_MAX(maxLen, -1), cvtRes, reqSize, nullptr, nullptr);
    if (res == 0) {
        free(cvtRes);

        *sizePtr = 0;
        return nullptr;
    }

    *sizePtr    = res     * sizeof *cvtRes;
    *memSizePtr = reqSize * sizeof *cvtRes;
    return cvtRes;
}


#endif /* defined KI_PLATFORM_WINDOWS */


