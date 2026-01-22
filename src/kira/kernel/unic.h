/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                  *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

/**
 * \file  unic.h
 * \brief defines some helper routines for common Unicode- and UTF-8 operations
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiIsUnicodeWhitespace(KiTUint32 codepoint);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiIsValidUnicodeCodepoint(KiTUint32 codepoint);
/**
 */
KI_NATIVE KI_API KiTUint32 KI_CALL KiToUnicodeCodepointFromUtf8(KiTChar const *u8sPtr, KiTBool *errPtr);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiIsValidUnicodeIdentifierStartCharacter(KiTUint32 codepoint);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiIsValidUnicodeIdentifierContinuationCharacter(KiTUint32 codepoint);

/**
 */
KI_NATIVE KI_API KiTSize KI_CALL KiGetUtf8CharacterSize(KiTChar const *u8sPtr);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiIsUtf8StringValid(KiTChar const *u8sPtr);
/**
 */
KI_NATIVE KI_API KiTChar *KI_CALL KiNextUtf8Character(KiTChar const *u8sStr);

/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiReadAndNormalizeUtf8File(KiTChar const *filePath, KiTChar **resPtr);


