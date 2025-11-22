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
 * \file  sync.h
 * \brief defines the APIs for kernel-level synchronization primitives on which the user-level synchronization
 *        primitives are also based on
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE typedef struct KiSKrnlRWLock KiSKrnlRWLock;


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlRWLockCreate(KiSKrnlRWLock **resPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlRWLockDestroy(KiSKrnlRWLock *rwLockPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlRWLockAcquireRead(KiSKrnlRWLock *rwLockPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlRWLockReleaseRead(KiSKrnlRWLock *rwLockPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlRWLockAcquireWrite(KiSKrnlRWLock *rwLockPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlRWLockReleaseWrite(KiSKrnlRWLock *rwLockPtr);


