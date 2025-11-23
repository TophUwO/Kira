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
KI_NATIVE typedef struct KiSString KiSString;


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringCreate(KiTChar const *srcStr, KiSString **resPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlStringDestroy(KiSString *strPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringAssign(KiSString *strPtr, KiTChar const *srcPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringDuplicate(KiSString const *srcPtr, KiSString **resPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringConcat(KiSString *strPtr, KiTChar const *srcStr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStringPushPathComponent(
    KiSString *strPtr,
    KiTChar const *pathCompPtr
);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlStringPopPathComponent(KiSString *strPtr);
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiKrnlStringReplaceChar(
    KiSString *strPtr,
    KiTChar ch2Replace,
    KiTChar replaceWith
);
/**
 */
KI_NATIVE extern KiTChar const *KI_CALL KiKrnlStringCStr(KiSString const *strPtr);
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiKrnlStringSize(KiSString const *strPtr);
/**
 */
KI_NATIVE extern KiTChar const *KI_CALL KiKrnlStringFindChar(KiSString const *strPtr, KiTChar ch, KiTBool isRev);


