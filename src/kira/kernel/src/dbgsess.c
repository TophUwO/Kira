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
 * \file  dbgsess.c
 * \brief implements the debug-session management routines
 */


/* stdlib includes */
#include <string.h>

/* Kira includes */
#include <kira/kernel/dbg.h>
#include <kira/kernel/except.h>


/** \cond INTERNAL */
/**
 */
static KiSDebugOptions gl_DebugOptions;


/**
 */
static KiTVoid KI_CALL KiInternal_DbgOnAssertFailedHandler(KiSException const *excPtr, KiTVoid *extraParam) {
    KI_ASSERT(excPtr != nullptr, KiErr_InParameter);
    KI_UNREFERENCED_PARAMETER(extraParam);

    KiDebugTerminateProcess((KiSDebugTerminationContext const *)excPtr->mp_excDataPtr);
}
/** \endcond */


KiEErrorCode KI_CALL KiStartDebugSession(KiSDebugOptions const *dbgOpt) {
    KI_ASSERT(dbgOpt != nullptr, KiErr_InParameter);
    
    /* Reset debug options. */
    memset(&gl_DebugOptions, 0, sizeof gl_DebugOptions);
    /* Initialize debug options. */
    memcpy_s(&gl_DebugOptions, sizeof gl_DebugOptions, dbgOpt, dbgOpt->m_structSize);

    /* Intitialize debug subsystems. */
    KiTBool res = KI_TRUE;
    {
        /* Register our exception handler for assertions. */
        if (gl_DebugOptions.m_registerOnAssertHandler)
            if (!KiSetExceptionHandler(KiExcTy_AssertionFailed, &KiInternal_DbgOnAssertFailedHandler))
                return KiErr_IllegalSystemState;
    }

    /* All good. */
    return KiErr_Ok;
}

KiTVoid KI_CALL KiStopDebugSession(KiTVoid) {
    /* Unregister our exception handler for assertions. */
    if (gl_DebugOptions.m_registerOnAssertHandler)
        KiSetExceptionHandler(KiExcTy_AssertionFailed, nullptr);
}

KiSDebugOptions const *KI_CALL KiGetDebugOptions(KiTVoid) {
    return &gl_DebugOptions;
}


