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
 * \file  error.h
 * \brief defines global error codes used by Kira's APIs plus conversion- and translation services
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiRegisterErrorStringifier(KiIErrorStringifier *errStrRef, KiTUint32 *offsetPtr);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiUnregisterErrorStringifier(KiTUint32 offset);
/**
 */
KI_NATIVE KI_API KiSStringView const *KI_CALL KiQueryErrorString(KiEErrorCode errorCode);
/**
 */
KI_NATIVE KI_API KiSStringView const *KI_CALL KiQueryErrorBrief(KiEErrorCode errorCode);
/**
 */
KI_NATIVE KI_API KiSStringView const *KI_CALL KiQueryErrorDetails(KiEErrorCode errorCode);

/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiNativeErrorCodeToKiraErrorCode(KiTInt32 ntErrorCode);
/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiErrnoToKiraErrorCode(KiTInt32 errnoCode);


