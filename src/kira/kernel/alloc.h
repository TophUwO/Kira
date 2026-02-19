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
 * \file  alloc.h
 * \brief defines the public API for Kira's kernel-level pool allocator
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiKrnlAllocateBlock(KiTSize sizeBytes, KiTSize alignBytes, KiTVoid **resPtr);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiKrnlFreeBlock(KiTVoid *blockPtr);
/**
 */
KI_NATIVE KI_API KiTSize KI_CALL KiKrnlGetBlockSize(KiTVoid const *blockPtr);


