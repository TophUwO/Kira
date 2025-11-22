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
 * \file  except.c
 * \brief implements the kernel exception handling system
 */


/* stdlib includes */
#include <stdatomic.h>

#include <string.h>

/* Kira includes */
#include <kira/kernel/except.h>

#include <kira/kernel/int/sync.h>
#include <kira/kernel/int/krnlmod.h>

#include <kira/dbg/dbg.h>


/** \cond INTERNAL */
KI_NATIVE typedef struct KiSKrnlExceptionSystemState {
    KiTAtomicFlag              m_initFlag;
    KiTKrnlExceptionHandlerCb  ma_handlerTable[__KiKrnlExcTy_Count__];
    KiSKrnlRWLock             *mp_rwLock;
} KiSKrnlExceptionSystemState;
/**
 */
static KiSKrnlExceptionSystemState gl_ExceptionSystemState = {};


/**
 */
static KiTVoid KI_CALL KiInternal_KrnlExcHndSystemPanic(KiSKrnlException const *excPtr, KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);
    KI_UNREFERENCED_PARAMETER(excPtr);

    /* Stub. */
    return;
}


/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(ExceptionHandlingSystem)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    if (atomic_fetch_or(&gl_ExceptionSystemState.m_initFlag, KI_TRUE) == KI_FALSE) {
        /* Reset state. */
        memset(&gl_ExceptionSystemState, 0, sizeof gl_ExceptionSystemState);

        /* Initialize the handler table. */
        for (KiTIndex i = 0; i < KI_COUNTOF(gl_ExceptionSystemState.ma_handlerTable); i++)
            gl_ExceptionSystemState.ma_handlerTable[i] = &KiInternal_KrnlExcHndSystemPanic;

        /* Create rwlock. */
        KiEErrorCode errorCode = KiKrnlRWLockCreate(&gl_ExceptionSystemState.mp_rwLock);
        {
            if (errorCode != KiErr_Ok) {
                atomic_store(&gl_ExceptionSystemState.m_initFlag, KI_FALSE);

                return errorCode;
            }
        }

        /* All good. */
        return KiErr_Ok;
    }

    /* Incorrect init state. */
    return KiErr_IncorrectInitState;
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(ExceptionHandlingSystem)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    if (atomic_fetch_and(&gl_ExceptionSystemState.m_initFlag, KI_FALSE) == KI_TRUE) {
        /* Destroy rwlock. */
        KiKrnlRWLockDestroy(gl_ExceptionSystemState.mp_rwLock);

        return KiErr_Ok;
    }

    /* Incorrect init state. */
    return KiErr_IncorrectInitState;
}
/** \endcond */


KiTVoid KI_CALL KiKrnlThrowException(KiSKrnlException const *excPtr, KiTVoid *extraParam) {
    if (atomic_load(&gl_ExceptionSystemState.m_initFlag) == KI_FALSE)
        return;

    KiTKrnlExceptionHandlerCb fnExcHandler = &KiInternal_KrnlExcHndSystemPanic;

    /* Select callback. */
    KiKrnlRWLockAcquireWrite(gl_ExceptionSystemState.mp_rwLock);
    {
        if (KI_INRANGE_INCL(excPtr->m_excType, KiKrnlExcTy_Invalid, __KiKrnlExcTy_Count__ - 1))
            fnExcHandler = gl_ExceptionSystemState.ma_handlerTable[excPtr->m_excType];
    }
    KiKrnlRWLockReleaseWrite(gl_ExceptionSystemState.mp_rwLock);

    /* Invoke exception handler. */
    (*fnExcHandler)(excPtr, extraParam);
}

KiTBool KI_CALL KiKrnlSetExceptionHandler(KiSKrnlExceptionType excType, KiTKrnlExceptionHandlerCb fnHnd) {
    if (atomic_load(&gl_ExceptionSystemState.m_initFlag) == KI_FALSE)
        return KI_FALSE;

    /* Check if the type is in range. */
    if (!KI_INRANGE_INCL(excType, KiKrnlExcTy_Invalid, __KiKrnlExcTy_Count__ - 1))
        return KI_FALSE;

    /* Update handler table. */
    KiKrnlRWLockAcquireWrite(gl_ExceptionSystemState.mp_rwLock);
    {
        /* If there is already a handler registered for the current exception type, we reject the new one. */
        if (gl_ExceptionSystemState.ma_handlerTable[excType] != &KiInternal_KrnlExcHndSystemPanic) {
            KiKrnlRWLockReleaseWrite(gl_ExceptionSystemState.mp_rwLock);

            return KI_FALSE;
        }

        /* Update. */
        gl_ExceptionSystemState.ma_handlerTable[excType] = fnHnd != nullptr ? fnHnd : &KiInternal_KrnlExcHndSystemPanic;
    }
    KiKrnlRWLockReleaseWrite(gl_ExceptionSystemState.mp_rwLock);

    /* All good. */
    return KI_TRUE;
}


KiTSize KI_CALL KiKrnlGetMaximumExceptionHandlerCount(KiTVoid) {
    /* Currently, we only allow one handler. */
    return 1;
}


/** \cond */
KI_KRNLMOD_DEFINE(ExceptionHandlingSystem) {
    .m_structSize = sizeof(KiSKrnlModuleInfo),
    .m_modUuid    = {},
    .m_modId      = KI_MAKE_STRING_VIEW("exception handling system"),
    .m_modFlags   = 0,
    
    .mp_fnInit    = &KI_KRNLMOD_INITFN(ExceptionHandlingSystem),
    .mp_fnUninit  = &KI_KRNLMOD_UNINITFN(ExceptionHandlingSystem)
};
/** \endcond */


