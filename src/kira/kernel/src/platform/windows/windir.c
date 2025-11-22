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
#if (defined KI_PLATFORM_WINDOWS)


/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/kernel/reg.h>

#include <kira/kernel/int/platform.h>

#include <kira/dbg/dbg.h>


KiTVoid KI_CALL KiPlatform_FreeString(KiTVoid *strPtr) {
    if (strPtr == nullptr)
        return;

    free(strPtr);
}


KiTChar *KI_CALL KiPlatform_GetCurrentWorkingDirectory(KiTVoid) {
    /* Get size of the buffer needed to store the working directory. */
    KiTChar *resPtr;
    KiTSize const reqSize = (KiTSize)GetCurrentDirectoryW(0, nullptr) + 1;
    if (reqSize > 0) {
        /* Allocate temp buffer. */
        WCHAR *tmpBuf = malloc(reqSize * sizeof *tmpBuf);
        if (tmpBuf == nullptr)
            return nullptr;
        /* Get path. */
        GetCurrentDirectoryW(reqSize, tmpBuf);

        /* Convert to native Kira encoding. */
        resPtr = KiPlatform_CreateFromNativeEncoding(tmpBuf);
        free(tmpBuf);
    }

    return resPtr;
}

KiEErrorCode KI_CALL KiPlatform_SetCurrentWorkingDirectory(KiTChar const *newWdPath) {
    KI_ASSERT(newWdPath != nullptr, KiErr_InParameter);
    KI_ASSERT(*newWdPath != '\0',   KiErr_InParameter);

    /* Convert to native encoding. */
    BOOL res;
    KiTVoid const *ntEncPath = KiPlatform_CreateFromKiraEncoding(newWdPath);
    {
        if (ntEncPath == nullptr)
            return KiErr_EncodingError;

        /* Set the directory. */
        res = SetCurrentDirectoryW(ntEncPath);
    }
    KiPlatform_FreeString((KiTVoid *)ntEncPath);

    return res == 0
        ? KiKrnlNativeErrorCodeToKiraErrorCode((KiTInt32)GetLastError())
        : KiErr_Ok
    ;
}

KiTChar *KI_CALL KiPlatform_GetApplicationRootDirectory(KiTVoid) {
    /*
     * Get the required size for the file name. This is a horrifyingly shitty trick to get a dynamically sized buffer
     * with the module file name on Windows ... the GetModuleFileName*()-API is just such bullshit.
     */
    DWORD  reqSize = MAX_PATH;
    WCHAR *pathBuf = nullptr;
    do {
        /* Allocate a buffer. */
        pathBuf = malloc(sizeof *pathBuf * reqSize);
        if (pathBuf == nullptr)
            return nullptr;

        /* Check if the buffer is wide enough for the entire path to fit in there. */
        DWORD const writtenCh = GetModuleFileNameW(nullptr, pathBuf, reqSize);
        if (writtenCh == 0) {
            free(pathBuf);

            return nullptr;
        } else if (writtenCh < reqSize + 1) {
            /* Great. It fits. */
            break;
        }

        /* Does not fit. Widen the buffer. */
        free(pathBuf);
        reqSize <<= 1;
    } while (KI_TRUE);

    /*
     * Awesome. After all this fuckery up top, we can convert the full path to a UTF-8 string. After this, the original
     * string is not needed any longer.
     */
    KiTChar *u8FullPath = KiPlatform_CreateFromNativeEncoding(pathBuf);
    {
        KiPlatform_FreeString(pathBuf);
        if (u8FullPath == nullptr)
            return nullptr;

        /* Convert to Kira ('/') separators. */
        KiPlatform_CanonicalizeSeparators(u8FullPath);
        /* Erase the last path component so that we are left with the runtime path. */
        KiTChar *lastSep = strrchr(u8FullPath, '/');
        if (lastSep != nullptr)
            memset(lastSep, 0, strlen(lastSep) * sizeof *lastSep);
    }

    /* All good. */
    return u8FullPath;
}

KiTVoid KI_CALL KiPlatform_CanonicalizeSeparators(KiTChar *bufPtr) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InOutParameter);

    KiTSize const bufLen = strlen(bufPtr);
    {
        for (KiTSize i = 0; i < bufLen; i++)
            if (bufPtr[i] == '\\')
                bufPtr[i] = '/';
    }
}
#endif /* KI_PLATFORM_WINDOWS */


