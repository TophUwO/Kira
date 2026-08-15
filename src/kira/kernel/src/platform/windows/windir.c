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
 * \file  windir.c
 * \brief implements Windows(R)-specific routines regarding directory handling
 */
#if (defined KI_PLATFORM_WINDOWS)


/* Windows includes */
#include <windows.h>
#include <shlwapi.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/int/platform.h>

/* static libraries */
#pragma comment (lib, "shlwapi.lib")


KiTChar *KI_CALL KiPlatform_AllocateString(KiTSize sizeInBytes) {
    KI_ASSERT(sizeInBytes > 0, KiErr_InParameter);

    return (KiTChar *)malloc(sizeInBytes);
}

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
        resPtr = KiPlatform_CreateFromNativeEncoding(tmpBuf, -1, KI_DONTCARE(KiTSize), KI_DONTCARE(KiTSize));
        free(tmpBuf);
    }

    return resPtr;
}

KiEErrorCode KI_CALL KiPlatform_SetCurrentWorkingDirectory(KiTChar const *newWdPath) {
    KI_ASSERT(newWdPath != nullptr, KiErr_InParameter);
    KI_ASSERT(*newWdPath != '\0',   KiErr_InParameter);

    /* Convert to native encoding. */
    BOOL res;
    KiTVoid const *ntEncPath = KiPlatform_CreateFromKiraEncoding(newWdPath, -1, KI_DONTCARE(KiTSize), KI_DONTCARE(KiTSize));
    {
        if (ntEncPath == nullptr)
            return KiErr_EncodingError;

        /* Set the directory. */
        res = SetCurrentDirectoryW(ntEncPath);
    }
    KiPlatform_FreeString((KiTVoid *)ntEncPath);

    return res == 0
        ? KiNativeErrorCodeToKiraErrorCode((KiTInt32)GetLastError())
        : KiErr_Ok
    ;
}

KiTChar *KI_CALL KiPlatform_GetApplicationRootDirectory(KiTSize *sizePtr, KiTSize *lenPtr) {
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);
    KI_ASSERT(lenPtr != nullptr,  KiErr_OutParameter);

    SetLastError(0);
    /*
     * Get the required size for the file name. This is a horrifyingly shitty trick to get a dynamically sized buffer
     * with the module file name on Windows ... the GetModuleFileName*()-API is just such bullshit.
     */
    DWORD  reqSize   = MAX_PATH;
    DWORD  writtenCh = 0;
    DWORD  errCode   = 0;
    WCHAR *pathBuf   = nullptr;
    do {
        /* Allocate a buffer. */
        pathBuf = (WCHAR *)KiPlatform_AllocateString(sizeof *pathBuf * reqSize);
        if (pathBuf == nullptr)
            return nullptr;

        /* Check if the buffer is wide enough for the entire path to fit in there. */
        writtenCh = GetModuleFileNameW(nullptr, pathBuf, reqSize);
        if (writtenCh == 0 || (errCode = GetLastError()) != 0 && errCode != ERROR_INSUFFICIENT_BUFFER) {
            KiPlatform_FreeString(pathBuf);

            *sizePtr = 0;
            *lenPtr  = 0;
            return nullptr;
        } else if (errCode == 0) {
            /* Great. It fits. */
            break;
        }

        /* Does not fit. Widen the buffer. */
        KiPlatform_FreeString(pathBuf);
        reqSize <<= 1;
    } while (KI_TRUE);

    /*
     * Awesome. After all this fuckery up top, we can convert the full path to a UTF-8 string. After this, the original
     * string is not needed any longer.
     */
    KiTChar *lastSep    = nullptr;
    KiTChar *u8FullPath = KiPlatform_CreateFromNativeEncoding(pathBuf, -1, lenPtr, KI_DONTCARE(KiTSize));
    {
        KiPlatform_FreeString(pathBuf);
        if (u8FullPath == nullptr) {
            *sizePtr = 0;
            *lenPtr  = 0;

            return nullptr;
        }

        /* Convert to Kira ('/') separators. */
        KiPlatform_CanonicalizeSeparators(u8FullPath);
        /* Erase the last path component so that we are left with the runtime path. */
        if ((lastSep = strrchr(u8FullPath, '/')) != nullptr)
            *lastSep = '\0';
    }

    *sizePtr = *lenPtr;
    *lenPtr  = (KiTOffset)lastSep - (KiTOffset)u8FullPath; /* We want length in bytes without NUL-terminator. */
    return u8FullPath;
}

KiTChar KI_CALL KiPlatform_GetPathSeparator(KiTVoid) {
    return '\\';
}

KiTChar *KI_CALL KiPlatform_CanonicalizeSeparators(KiTChar *pathStr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InOutParameter);

    KiTChar *sepPtr = strchr(pathStr, KiPlatform_GetPathSeparator());
    while (sepPtr != nullptr) {
        *sepPtr = '/';

        sepPtr = strchr(sepPtr + 1, KiPlatform_GetPathSeparator());
    }

    return pathStr;
}

KiTBool KI_CALL KiPlatform_PathExists(KiTChar const *pathStr, KiTBool isDir) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);

    KiTBool res;
    WCHAR *cvtPath = KiPlatform_CreateFromKiraEncoding(pathStr, -1, KI_DONTCARE(KiTSize), KI_DONTCARE(KiTSize));
    {
        if (cvtPath == nullptr)
            return KI_FALSE;

        if (isDir == KI_TRUE)
            res = PathIsDirectoryW(cvtPath) != KI_FALSE;
        else {
            DWORD const attr = GetFileAttributesW(cvtPath);

            res = attr ^ INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == KI_FALSE;
        }
    }
    KiPlatform_FreeString(cvtPath);

    return res;
}

KiTBool KI_CALL KiPlatform_IsPathRelative(KiTChar const *pathStr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);

    KiTBool res;
    WCHAR *cvtPath = KiPlatform_CreateFromKiraEncoding(pathStr, -1, KI_DONTCARE(KiTSize), KI_DONTCARE(KiTSize));
    {
        if (cvtPath == nullptr)
            return KI_FALSE;

        res = (KiTBool)PathIsRelativeW(cvtPath);
    }
    KiPlatform_FreeString(cvtPath);

    return res;
}


#endif /* (defined KI_PLATFORM_WINDOWS) */
