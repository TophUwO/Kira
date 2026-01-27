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
 * \file  string.c
 * \brief implements the kernel-level lightweight dynamic string type, based on KiSBuffer
 */


/* stdlib includes */
#include <stdlib.h>

#include <string.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/int/string.h>
#include <kira/kernel/int/buffer.h>
#include <kira/kernel/int/platform.h>


KiEErrorCode KI_CALL KiCreateString(KiTChar const *srcStr, KiSString **resPtr) {
    KI_ASSERT(srcStr != nullptr, KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    KiEErrorCode errCode = KiCreateBuffer(0, (KiSBuffer **)resPtr);
    {
        if (errCode != KiErr_Ok)
            return errCode;

        /* If the string is nullptr or empty, we leave the buffer uninitialized. */
        if (srcStr != nullptr && *srcStr != '\0') {
            KiEErrorCode errCode = KiWriteBufferData((KiSBuffer *)*resPtr, srcStr, strlen(srcStr) + 1);

            if (errCode != KiErr_Ok) {
                KiDestroyBuffer((KiSBuffer *)*resPtr);

                *resPtr = nullptr;
                return errCode;
            }
        }
    }

    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiCreateStringApplicationRootDir(KiSString **resPtr) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    KiTSize size, len;
    KiTChar const *rootDir = KiPlatform_GetApplicationRootDirectory(&size, &len);
    if (rootDir == nullptr) {
        *resPtr = nullptr;

        return KiErr_GetSystemPath;
    }

    KiEErrorCode errCode = KiCreateBuffer(0, (KiSBuffer **)resPtr);
    {
        if (errCode != KiErr_Ok) {
            free((KiTVoid *)rootDir);

            *resPtr = nullptr;
            return errCode;
        }

        KiAttachBuffer((KiSBuffer *)*resPtr, rootDir, size, len + 1);
    }

    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiDuplicateString(KiSString const *srcPtr, KiSString **resPtr) {
    KI_ASSERT(srcPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    KiSBuffer *srcAsBuf = (KiSBuffer *)srcPtr;
    {
        KiEErrorCode errCode = KiCreateBuffer(KiGetBufferSize((KiSBuffer *)srcPtr), (KiSBuffer **)resPtr);
        if (errCode != KiErr_Ok)
            return errCode;

        errCode = KiWriteBufferData((KiSBuffer *)*resPtr, KiGetBufferPointer(srcAsBuf, 0), KiGetBufferSize(srcAsBuf));
        if (errCode != KiErr_Ok) {
            KiDestroyBuffer((KiSBuffer *)*resPtr);

            *resPtr = nullptr;
            return errCode;
        }
    }

    return KiErr_Ok;
}

KiTVoid KI_CALL KiDestroyString(KiSString *strPtr) {
    if (strPtr == nullptr)
        return;

    KiDestroyBuffer((KiSBuffer *)strPtr);
}

KiTVoid KI_CALL KiClearString(KiSString *strPtr, KiTBool isSecure) {
    KI_ASSERT(strPtr != nullptr, KiErr_InOutParameter);

    /* Do not use KiFillBuffer() because it's simply too slow for this operation. */
    if (isSecure) {
        KiTSize const bufSize = KiGetBufferSize((KiSBuffer const *)strPtr);
        
        memset((KiTVoid *)KiGetBufferPointer((KiSBuffer const *)strPtr, 0), 0, bufSize);
    } else
        *(KiTChar *)KiGetBufferPointer((KiSBuffer const *)strPtr, 0) = '\0';

    KiSeekBufferPosition((KiSBuffer *)strPtr, KI_SEEK_BEGIN);
}


KiEErrorCode KI_CALL KiAssignToString(KiSString *strPtr, KiTChar const *srcPtr) {
    KI_ASSERT(strPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(srcPtr != nullptr, KiErr_InParameter);

    /* Seek to the beginning so that we start writing to the first byte of the buffer. */
    KiSeekBufferPosition((KiSBuffer *)strPtr, KI_SEEK_BEGIN);

    KiEErrorCode errCode = KiWriteBufferData((KiSBuffer *)strPtr, srcPtr, strlen(srcPtr) + 1);
    {
        if (errCode != KiErr_Ok)
            return errCode;

        /* Zero the remainder of the memory block currently allocated by the buffer. */
        KiTOffset const currPos = KiGetBufferPosition((KiSBuffer const *)strPtr);
        {
            memset(
                (KiTVoid *)KiGetBufferPointer((KiSBuffer const *)strPtr, currPos),
                0,
                KiGetBufferSize((KiSBuffer const *)strPtr) - currPos
            );
        }
    }

    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiAppendToString(KiSString *strPtr, KiTChar const *srcStr) {
    KI_ASSERT(strPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(srcStr != nullptr, KiErr_InParameter);

    return KiWriteBufferData((KiSBuffer *)strPtr, srcStr, strlen(srcStr) + 1);
}

KiEErrorCode KI_CALL KiPushPathComponent(KiSString *strPtr, KiTChar pathSep, KiTChar const *pathCompPtr) {
    KI_ASSERT(strPtr != nullptr,      KiErr_InOutParameter);
    KI_ASSERT(pathCompPtr != nullptr, KiErr_InParameter);

    KiTOffset const oldOff = KiGetBufferPosition((KiSBuffer const *)strPtr);
    {
        KiEErrorCode errCode = KiErr_Ok;

        /* Write separator. */
        KiSeekBufferPosition((KiSBuffer *)strPtr, KI_MAX(0, oldOff - 1));
        {
            errCode = KiAppendToString(strPtr, (KiTChar const[2]){ pathSep, '\0' });

            if (errCode != KiErr_Ok)
                return errCode;
        }
        /*
         * Because the separator was written as string, it also appended a NUL-terminator. This means we must overwrite
         * it as well. Since the old offset effectively points to the byte after the old NUL-terminator and the old
         * NUL-terminator has been overwritten by the separator, seeking to the old offset effectively places us right
         * after the separator.
         */
        KiSeekBufferPosition((KiSBuffer *)strPtr, oldOff);

        /* Finally write the path component. */
        if ((errCode = KiAppendToString(strPtr, pathCompPtr)) != KiErr_Ok) {
            /* We do not need to erase more because if KiAppendToString() fails, nothing has been written. */
            *(KiTChar *)KiGetBufferPointer((KiSBuffer const *)strPtr, oldOff) = '\0';

            KiSeekBufferPosition((KiSBuffer *)strPtr, oldOff);
            return errCode;
        }
    }

    return KiErr_Ok;
}

KiTVoid KI_CALL KiPopPathComponent(KiSString *strPtr, KiTChar pathSep) {
    KI_ASSERT(strPtr != nullptr, KiErr_InOutParameter);

    /* Find last separator. */
    KiTVoid *lastSepPtr = strrchr(KiGetBufferPointer((KiSBuffer const *)strPtr, KI_SEEK_BEGIN), pathSep);
    if (lastSepPtr == nullptr)
        return;

    /*
     * Zero everything between the current offset and the position of the last separator. Lastly, seek to the new end of
     * the string.
     */
    memset(lastSepPtr, 0, strlen(lastSepPtr));
    KiSeekBufferPosition(
        (KiSBuffer *)strPtr,
        (KiTIntptr)lastSepPtr - (KiTIntptr)KiGetBufferPointer((KiSBuffer const *)strPtr, KI_SEEK_BEGIN) + 1
    );
}


KiTChar const *KI_CALL KiGetCString(KiSString const *strPtr) {
    KI_ASSERT(strPtr != nullptr, KiErr_InParameter);

    return KiGetBufferPointer((KiSBuffer const *)strPtr, 0);
}

KiTSize KI_CALL KiGetStringSize(KiSString const *strPtr) {
    KI_ASSERT(strPtr != nullptr, KiErr_InParameter);

    return KiGetBufferSize((KiSBuffer const *)strPtr);
}


