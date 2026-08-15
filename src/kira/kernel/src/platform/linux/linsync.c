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
 * \file  linsync.c
 * \brief implements Linux-specific low-level synchronization routines
 */


#if (defined KI_PLATFORM_LINUX)
/* stdlib includes */
#include <stdlib.h>

/* Linux includes */
#include <pthread.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


/* Need to verify that KiTRWLockHandle is large enough to hold a pointer. */
KI_VERIFY_TYPE(pthread_rwlock_t *, KiTRWLockHandle);


KiEErrorCode KI_CALL KiVirtual_RWLockCreate(KiTRWLockHandle *resPtr) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutParameter);

    /* (1) Allocate the structure. */
    *resPtr = (KiTRWLockHandle)malloc(sizeof(pthread_rwlock_t));
    if (*resPtr == (KiTRWLockHandle)nullptr)
        return KiErr_MemoryAllocation;

    /* (2) Initialize the rwlock. */
    int const res = pthread_rwlock_init((pthread_rwlock_t *)*resPtr, nullptr);
    if (res != 0) {
        free((KiTVoid *)*resPtr);

        *resPtr = 0;
        return KiNativeErrorCodeToKiraErrorCode(res);
    }

    /* All good. */
    return KiErr_Ok;
}

KiTVoid KI_CALL KiVirtual_RWLockDestroy(KiTRWLockHandle *rwLockPtr) {
    if (rwLockPtr == nullptr || *rwLockPtr == 0)
        return;

    KI_IGNORE_RETURN_VALUE(pthread_rwlock_destroy((pthread_rwlock_t *)*rwLockPtr));

    free((KiTVoid *)*rwLockPtr);
    *rwLockPtr = 0;
}

KiTVoid KI_CALL KiVirtual_RWLockAcquireRead(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(*rwLockPtr != 0,      KiErr_InOutParameter);

    KI_IGNORE_RETURN_VALUE(pthread_rwlock_rdlock((pthread_rwlock_t *)*rwLockPtr));
}

KiTVoid KI_CALL KiVirtual_RWLockReleaseRead(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(*rwLockPtr != 0,      KiErr_InOutParameter);

    KI_IGNORE_RETURN_VALUE(pthread_rwlock_unlock((pthread_rwlock_t *)*rwLockPtr));
}

KiTVoid KI_CALL KiVirtual_RWLockAcquireWrite(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(*rwLockPtr != 0,      KiErr_InOutParameter);

    KI_IGNORE_RETURN_VALUE(pthread_rwlock_wrlock((pthread_rwlock_t *)*rwLockPtr));
}

KiTVoid KI_CALL KiVirtual_RWLockReleaseWrite(KiTRWLockHandle *rwLockPtr) {
    KI_ASSERT(rwLockPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(*rwLockPtr != 0,      KiErr_InOutParameter);

    KI_IGNORE_RETURN_VALUE(pthread_rwlock_unlock((pthread_rwlock_t *)*rwLockPtr));
}
#endif /* (defined KI_PLATFORM_LINUX) */
