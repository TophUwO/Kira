/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/


/**
 * \file  sync.c
 * \brief implements the kernel-level synchronization primitives
 */


/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/reg.h>

#include <kira/kernel/int/sync.h>
#include <kira/kernel/int/platform.h>


KiEErrorCode KI_CALL KiCreateRWLock(KiTRWLockHandle *resPtr) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    return KiVirtual_RWLockCreate(resPtr);
}

KiTVoid KI_CALL KiDestroyRWLock(KiTRWLockHandle *rwLockPtr) {
    KiVirtual_RWLockDestroy(rwLockPtr);
}

KiTVoid KI_CALL KiAcquireRead(KiTRWLockHandle *rwLockPtr) {
    KiVirtual_RWLockAcquireRead(rwLockPtr);
}

KiTVoid KI_CALL KiReleaseRead(KiTRWLockHandle *rwLockPtr) {
    KiVirtual_RWLockReleaseRead(rwLockPtr);
}

KiTVoid KI_CALL KiAcquireWrite(KiTRWLockHandle *rwLockPtr) {
    KiVirtual_RWLockAcquireWrite(rwLockPtr);
}

KiTVoid KI_CALL KiReleaseWrite(KiTRWLockHandle *rwLockPtr) {
    KiVirtual_RWLockReleaseWrite(rwLockPtr);
}
