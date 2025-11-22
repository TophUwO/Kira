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
 * \brief implements the kernel-level lightweight dynamic string type
 */


/* stdlib includes */
#include <stdlib.h>

#include <string.h>

/* Kira includes */
#include <kira/kernel/reg.h>

#include <kira/kernel/int/string.h>

#include <kira/dbg/dbg.h>


struct KiSKrnlString {
    KiTSize  m_strSize;
    KiTSize  m_strCap;

    KiTChar *mp_byteArrPtr;
};


KiEErrorCode KI_CALL KiKrnlStringCreate(KiTChar const *srcStr, KiSKrnlString **resPtr) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    /* Allocate structure. */
    *resPtr = malloc(sizeof **resPtr);
    if (*resPtr == nullptr)
        return KiErr_MemoryAllocation;

    /* Initialize with string if necessary. */
    if (srcStr != nullptr && *srcStr != '\0') {
        KiTSize const srcSize = strlen(srcStr);
        {
            /* Allocate string buffer. */
            (*resPtr)->mp_byteArrPtr = malloc((srcSize + 1) * sizeof *(*resPtr)->mp_byteArrPtr);
            if ((*resPtr)->mp_byteArrPtr == nullptr) {
                free(*resPtr);

                *resPtr = nullptr;
                return KiErr_MemoryAllocation;
            }

            /* Copy. */
            memcpy_s((*resPtr)->mp_byteArrPtr, srcSize + 1, srcStr, srcSize + 1);
            **resPtr = (KiSKrnlString){
                .m_strSize     = srcSize,
                .m_strCap      = srcSize + 1,
                .mp_byteArrPtr = (*resPtr)->mp_byteArrPtr
            };
        }

        /* All good. */
        return KiErr_Ok;
    }

    /* Return empty string. */
    **resPtr = (KiSKrnlString){
        .m_strCap      = 0,
        .m_strSize     = 0,
        .mp_byteArrPtr = nullptr
    };
    return KiErr_Ok;
}

KiTVoid KI_CALL KiKrnlStringDestroy(KiSKrnlString *strPtr) {
    if (strPtr == nullptr)
        return;

    free(strPtr->mp_byteArrPtr);
    free(strPtr);
}

KiEErrorCode KI_CALL KiKrnlStringAssign(KiSKrnlString *strPtr, KiTChar const *srcPtr) {
    
}

KiEErrorCode KI_CALL KiKrnlStringDuplicate(KiSKrnlString const *srcPtr, KiSKrnlString **resPtr) {
    KI_ASSERT(srcPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);
    
    return KiKrnlStringCreate(KiKrnlStringCStr(srcPtr), resPtr);
}

KiEErrorCode KI_CALL KiKrnlStringConcat(KiSKrnlString *strPtr, KiTChar const *srcStr) {

}

KiEErrorCode KI_CALL KiKrnlStringPushPathComponent(KiSKrnlString *strPtr, KiTChar const *pathCompPtr) {

}

KiTVoid KI_CALL KiKrnlStringPopPathComponent(KiSKrnlString *strPtr) {

}

KiTSize KI_CALL KiKrnlStringReplaceChar(KiSKrnlString *strPtr, KiTChar ch2Replace, KiTChar replaceWith) {

}

KiTChar const *KI_CALL KiKrnlStringCStr(KiSKrnlString const *strPtr) {
    KI_ASSERT(strPtr != nullptr, KiErr_InParameter);

    return (KiTChar const *)strPtr->mp_byteArrPtr;
}

KiTSize KI_CALL KiKrnlStringSize(KiSKrnlString const *strPtr) {
    KI_ASSERT(strPtr != nullptr, KiErr_InParameter);

    return strPtr->m_strSize;
}

KiTChar const *KI_CALL KiKrnlStringFindChar(KiSKrnlString const *strPtr, KiTChar ch, KiTBool isRev) {

}


