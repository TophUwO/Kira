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
KI_NATIVE extern KiEErrorCode KI_CALL KiCreateString(KiTChar const *srcStr, KiSString **resPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiCreateStringApplicationRootDir(KiSString **resPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiDuplicateString(KiSString const *srcPtr, KiSString **resPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiDestroyString(KiSString *strPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiClearString(KiSString *strPtr, KiTBool isSecure);

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiAssignToString(KiSString *strPtr, KiTChar const *srcPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiAppendToString(KiSString *strPtr, KiTChar const *srcStr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPushPathComponent(KiSString *strPtr, KiTChar pathSep, KiTChar const *pathCompPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPopPathComponent(KiSString *strPtr, KiTChar pathSep);

/**
 */
KI_NATIVE extern KiTChar const *KI_CALL KiGetCString(KiSString const *strPtr);
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiGetStringSize(KiSString const *strPtr);


