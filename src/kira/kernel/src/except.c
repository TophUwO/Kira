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
#include <kira/dbg.h>

#include <kira/kernel/except.h>

#include <kira/kernel/int/sync.h>
#include <kira/kernel/int/krnlmod.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef struct KiSExceptionSystemState {
    KiTBool              m_isInit;
    KiFExceptionHandler  ma_handlerTable[__KiExcTy_Count__];
    KiTRWLockHandle      m_rwLock;
} KiSExceptionSystemState;

/**
 */
static KiSExceptionSystemState gl_ExceptionSystemState;


/**
 */
static KiTVoid KI_CALL KiInternal_ExcHndSystemPanic(KiSException const *excPtr, KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);
    KI_UNREFERENCED_PARAMETER(excPtr);

    /* Stub. */
    return;
}


/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(ExceptionHandlingSystem)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    if (gl_ExceptionSystemState.m_isInit == KI_FALSE) {
        /* Reset state. */
        memset(&gl_ExceptionSystemState, 0, sizeof gl_ExceptionSystemState);

        /* Initialize the handler table. */
        for (KiTIndex i = 0; i < KI_COUNTOF(gl_ExceptionSystemState.ma_handlerTable); i++)
            gl_ExceptionSystemState.ma_handlerTable[i] = &KiInternal_ExcHndSystemPanic;

        /* Create rwlock. */
        KiEErrorCode errorCode = KiCreateRWLock(&gl_ExceptionSystemState.m_rwLock);
        {
            if (errorCode != KiErr_Ok)
                return errorCode;
        }

        /* All good. */
        gl_ExceptionSystemState.m_isInit = KI_TRUE;
        return KiErr_Ok;
    }

    /* Incorrect init state. */
    return KiErr_IncorrectInitState;
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(ExceptionHandlingSystem)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    if (gl_ExceptionSystemState.m_isInit == KI_TRUE) {
        /* Destroy rwlock. */
        KiDestroyRWLock(&gl_ExceptionSystemState.m_rwLock);

        gl_ExceptionSystemState.m_isInit = KI_FALSE;
        return KiErr_Ok;
    }

    /* Incorrect init state. */
    return KiErr_IncorrectInitState;
}
/** \endcond */


KiTVoid KI_CALL KiRaiseException(KiSException const *excPtr, KiTVoid *extraParam) {
    if (gl_ExceptionSystemState.m_isInit == KI_FALSE)
        return;

    KiFExceptionHandler fnExcHandler = &KiInternal_ExcHndSystemPanic;

    /* Select callback. */
    KiAcquireWrite(&gl_ExceptionSystemState.m_rwLock);
    {
        if (KI_INRANGE_INCL(excPtr->m_excType, KiExcTy_Invalid, __KiExcTy_Count__ - 1))
            fnExcHandler = gl_ExceptionSystemState.ma_handlerTable[excPtr->m_excType];
    }
    KiReleaseWrite(&gl_ExceptionSystemState.m_rwLock);

    /* Invoke exception handler. */
    (*fnExcHandler)(excPtr, extraParam);
}

KiTBool KI_CALL KiSetExceptionHandler(KiEExceptionType excType, KiFExceptionHandler fnHnd) {
    if (gl_ExceptionSystemState.m_isInit == KI_FALSE)
        return KI_FALSE;

    /* Check if the type is in range. */
    if (!KI_INRANGE_INCL(excType, KiExcTy_Invalid, __KiExcTy_Count__ - 1))
        return KI_FALSE;

    /* Update handler table. */
    KiAcquireWrite(&gl_ExceptionSystemState.m_rwLock);
    {
        /* If there is already a handler registered for the current exception type, we reject the new one. */
        if (gl_ExceptionSystemState.ma_handlerTable[excType] != &KiInternal_ExcHndSystemPanic) {
            KiReleaseWrite(&gl_ExceptionSystemState.m_rwLock);

            return KI_FALSE;
        }

        /* Update. */
        gl_ExceptionSystemState.ma_handlerTable[excType] = fnHnd != nullptr ? fnHnd : &KiInternal_ExcHndSystemPanic;
    }
    KiReleaseWrite(&gl_ExceptionSystemState.m_rwLock);

    /* All good. */
    return KI_TRUE;
}


KiTSize KI_CALL KiGetMaximumExceptionHandlerCount(KiTVoid) {
    /* Currently, we only allow one handler. */
    return 1;
}


/** \cond */
KI_KRNLMOD(ExceptionHandlingSystem, {
    &KI_MAKE_UUID(0xFFFFFFFF, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC)
}, {
    .mp_modUuid  = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId    = &KI_MAKE_STRING_VIEW("exception handling system"),
    .m_modFlags  = 0
});
/** \endcond */


