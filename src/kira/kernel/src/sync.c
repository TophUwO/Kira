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
 * \file  sync.c
 * \brief implements the kernel-level synchronization primitives
 */


/* Kira includes */
#include <kira/error.h>

#include <kira/kernel/reg.h>

#include <kira/kernel/int/sync.h>
#include <kira/kernel/int/platform.h>

#include <kira/dbg/dbg.h>


KiEErrorCode KI_CALL KiKrnlRWLockCreate(KiSKrnlRWLock **resPtr) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    return KiVirtual_KrnlRWLockCreate(resPtr);
}

KiTVoid KI_CALL KiKrnlRWLockDestroy(KiSKrnlRWLock *rwLockPtr) {
    if (rwLockPtr == nullptr)
        return;

    KiVirtual_KrnlRWLockDestroy(rwLockPtr);
}

KiTVoid KI_CALL KiKrnlRWLockAcquireRead(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    KiVirtual_KrnlRWLockAcquireRead(rwLockPtr);
}

KiTVoid KI_CALL KiKrnlRWLockReleaseRead(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    KiVirtual_KrnlRWLockReleaseRead(rwLockPtr);
}

KiTVoid KI_CALL KiKrnlRWLockAcquireWrite(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    KiVirtual_KrnlRWLockAcquireWrite(rwLockPtr);
}

KiTVoid KI_CALL KiKrnlRWLockReleaseWrite(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    KiVirtual_KrnlRWLockReleaseWrite(rwLockPtr);
}


