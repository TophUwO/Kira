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
 * \file  winsync.c
 * \brief implements the kernel-level synchronization primitives for the Microsoft Windows(R) platform
 */


#if (defined KI_PLATFORM_WINDOWS)
/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/def.h>
#include <kira/dbg.h>

#include <kira/kernel/error.h>

#include <kira/kernel/int/sync.h>


/*
 * This code relies on the fact that an SRWLOCK is just a number the size of a pointer. This is unlikely to ever change,
 * but better be safe than sorry.
 */
KI_VERIFY_TYPE(SRWLOCK, KiTRWLockHandle);


KiEErrorCode KI_CALL KiVirtual_RWLockCreate(KiTRWLockHandle *resPtr) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    /* Initialize state. */
    InitializeSRWLock((SRWLOCK *)resPtr);

    /* All good. */
    return KiErr_Ok;
}

KiTVoid KI_CALL KiVirtual_RWLockDestroy(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    /* Stub because we don't need to explicitly destroy the SRWLOCK. */
    return;
}

KiTVoid KI_CALL KiVirtual_RWLockAcquireRead(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    AcquireSRWLockShared((SRWLOCK *)rwLockPtr);
}

KiTVoid KI_CALL KiVirtual_RWLockReleaseRead(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    ReleaseSRWLockShared((SRWLOCK *)rwLockPtr);
}

KiTVoid KI_CALL KiVirtual_RWLockAcquireWrite(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    AcquireSRWLockExclusive((SRWLOCK *)rwLockPtr);
}

KiTVoid KI_CALL KiVirtual_RWLockReleaseWrite(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    ReleaseSRWLockExclusive((SRWLOCK *)rwLockPtr);
}
#endif /* (defined KI_PLATFORM_WINDOWS) */
