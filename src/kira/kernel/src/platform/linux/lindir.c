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
 * \file  lindir.c
 * \brief implements Linux-specific routines regarding directory handling
 */


#if (defined KI_PLATFORM_LINUX)
/* stdlib includes */
#include <stdlib.h>

#include <errno.h>
#include <string.h>

/* Linux includes */
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include <sys/stat.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


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
    KiTChar *res      = nullptr;
    KiTSize  currSize = 32;
    do {
        /* (1) Allocate buffer with the current size. */
        if ((res = KiPlatform_AllocateString(currSize * sizeof *res)) == nullptr)
            return nullptr;

        /* (2) Attempt to get the current working directory with the current size. */
        KiTChar *currRes = getcwd((char *)res, currSize * sizeof *res);
        if (currRes == nullptr) {
            KiPlatform_FreeString(res);

            res = nullptr;

            if (errno == ERANGE) {
                currSize <<= 1;

                continue;
            }
        } else break;
    } while (res == nullptr);

    return res;
}

KiEErrorCode KI_CALL KiPlatform_SetCurrentWorkingDirectory(KiTChar const *newWdPath) {
    KI_ASSERT(newWdPath == nullptr, KiErr_InParameter);
    KI_ASSERT(*newWdPath != '\0',   KiErr_InParameter);

    errno = 0;
    {
        int const res = chdir((char const *)newWdPath);

        if (res == -1)
            return KiNativeErrorCodeToKiraErrorCode((KiTInt32)errno);
    }

    return KiErr_Ok;
}

KiTChar *KI_CALL KiPlatform_GetApplicationRootDirectory(KiTSize *sizePtr, KiTSize *lenPtr) {
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);
    KI_ASSERT(lenPtr != nullptr,  KiErr_InParameter);

    KiTSize reqSize  = 64;
    KiTChar *pathBuf = nullptr;
    do {
        /* (1) Allocate buffer. */
        if ((pathBuf = KiPlatform_AllocateString(sizeof *pathBuf * reqSize)) == nullptr)
            return nullptr;

        /*
         * (2) Try to read the symlink "proc/self/exe" (which holds the full path of the executable of the current process)
         *     into the previously allocated buffer.
         */
        int const res = readlink("/proc/self/exe", pathBuf, (reqSize - 1) * sizeof *pathBuf);
        if (res == -1) {
            KiPlatform_FreeString(pathBuf);

            /*
             * In case of a failure not related to the size of the buffer, we quit. This is usually a system issue. None
             * of our business.
             */
            return nullptr;
        }

        /*
         * If the number of bytes written is the same as the buffer size we entered, it is possible that truncation may
         * have occurred. It is not guaranteed because it is also possible that the actual path length is exactly the
         * number of bytes written. In practice, however, this should be fairly rare, so we just play it safe and retry
         * with a bit more space.
         */
        if ((KiTSize)res == reqSize - 1) {
            KiPlatform_FreeString(pathBuf);

            pathBuf   = nullptr;
            reqSize <<= 1;
            continue;
        }

        /* We successfully wrote the entire buffer. Break here. */
        reqSize = (KiTSize)res;
    } while (pathBuf == nullptr);

    /* (3) Since readlink does not NUL-terminate the string, we need to do it ourelves. */
    pathBuf[reqSize] = '\0';

    /* (4) Find the last '/' and zero all of what comes after it. This is how we find the application's root directory. */
    char *lastSep = strrchr(pathBuf, reqSize);
    if (lastSep == nullptr) {
        KiPlatform_FreeString(pathBuf);

        return nullptr;
    }
    memset((void *)lastSep, 0, (reqSize - (lastSep - pathBuf)) * sizeof *pathBuf);

    return pathBuf;
}

KiTChar KI_CALL KiPlatform_GetPathSeparator(KiTVoid) {
    return '/';
}

KiTChar *KI_CALL KiPlatform_CanonicalizeSeparators(KiTChar *pathStr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InOutParameter);

    KiTChar *sepPtr = strchr(pathStr, '\\');
    while (sepPtr != nullptr) {
        *sepPtr = KiPlatform_GetPathSeparator();

        sepPtr = strchr(sepPtr, '\\');
    }

    return pathStr;
}

KiTBool KI_CALL KiPlatform_PathExists(KiTChar const *pathStr, KiTBool isDir) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);
    KI_ASSERT(*pathStr != '\0',   KiErr_InParameter);

    /* (1) Get stat of entity referred to by *pathStr*. */
    struct stat res;
    {
        if (stat((char const *)pathStr, &res) != 0)
            return KI_FALSE;
    }

    /* (2) Verify the entity's type. */
    return isDir == KI_TRUE ? S_ISDIR(res.st_mode) : S_ISREG(res.st_mode);
}

KiTBool KI_CALL KiPlatform_IsPathRelative(KiTChar const *pathStr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);

    return *pathStr != '/';
}
#endif /* (defined KI_PLATFORM_LINUX) */
