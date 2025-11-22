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
 * \file  string.h
 * \brief defines the API for a kernel-level lightweight dynamic string type
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE typedef struct KiSKrnlString KiSKrnlString;


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringCreate(KiTChar const *srcStr, KiSKrnlString **resPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlStringDestroy(KiSKrnlString *strPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringAssign(KiSKrnlString *strPtr, KiTChar const *srcPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringDuplicate(KiSKrnlString const *srcPtr, KiSKrnlString **resPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringConcat(KiSKrnlString *strPtr, KiTChar const *srcStr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringPushPathComponent(
    KiSKrnlString *strPtr,
    KiTChar const *pathCompPtr
);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlStringPopPathComponent(KiSKrnlString *strPtr);
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiKrnlStringReplaceChar(
    KiSKrnlString *strPtr,
    KiTChar ch2Replace,
    KiTChar replaceWith
);
/**
 */
KI_NATIVE extern KiTChar const *KI_CALL KiKrnlStringCStr(KiSKrnlString const *strPtr);
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiKrnlStringSize(KiSKrnlString const *strPtr);
/**
 */
KI_NATIVE extern KiTChar const *KI_CALL KiKrnlStringFindChar(KiSKrnlString const *strPtr, KiTChar ch, KiTBool isRev);


