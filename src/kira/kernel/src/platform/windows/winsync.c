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
 * \file  winsync.c
 * \brief implements the kernel-level synchronization primitives for the Kicrosoft Windows(R) platform
 */
#if (defined KI_PLATFORM_WINDOWS)


/* Windows includes */
#include <windows.h>
#include <synchapi.h>

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/dbg.h>
#include <kira/kernel/error.h>

#include <kira/kernel/int/sync.h>


/**
 */
struct KiSKrnlRWLock {
    SRWLOCK m_rwLock; /**< internal rw-lock type */
};

KiEErrorCode KI_CALL KiVirtual_KrnlRWLockCreate(KiSKrnlRWLock **resPtr) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    /* Allocate memory. */
    *resPtr = malloc(sizeof **resPtr);
    if (*resPtr == nullptr)
        return KiErr_MemoryAllocation;

    /* Initialize state. */
    InitializeSRWLock(&(*resPtr)->m_rwLock);

    /* All good. */
    return KiErr_Ok;
}

KiTVoid KI_CALL KiVirtual_KrnlRWLockDestroy(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    /* Only free the struct as we don't need to explicitly destroy the SRWLOCK. */
    free(rwLockPtr);
}

KiTVoid KI_CALL KiVirtual_KrnlRWLockAcquireRead(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    AcquireSRWLockShared(&rwLockPtr->m_rwLock);
}

KiTVoid KI_CALL KiVirtual_KrnlRWLockReleaseRead(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    ReleaseSRWLockShared(&rwLockPtr->m_rwLock);
}

KiTVoid KI_CALL KiVirtual_KrnlRWLockAcquireWrite(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    AcquireSRWLockExclusive(&rwLockPtr->m_rwLock);
}

KiTVoid KI_CALL KiVirtual_KrnlRWLockReleaseWrite(KiSKrnlRWLock *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);

    ReleaseSRWLockExclusive(&rwLockPtr->m_rwLock);
}


#endif /* defined KI_PLATFORM_WINDOWS */


