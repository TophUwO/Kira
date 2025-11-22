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
 * \file  term.c
 * \brief implements the component that terminates the application in case a fatal error (in response to a failed
 *        \c KI_ASSERT(), for example) occurs 
 */


/* stdlib includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Kira includes */
#include <kira/kernel/dbg.h>

#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
static KiTChar constexpr gl_c_ErrorMsgFormat[] = 
    "A debug error occurred and the application was forced to halt:\n\n"
    "  Expr:\t%s\n"
    "  Code:\t%s (%u)\n"
    "  Desc:\t%s\n"
    "  File:\t%s\n"
    "  Line:\t%llu\n"
    "  Func:\t%s()\n\n"
    "%s\n\n"
    "This error signifies abnormal program termination. Please contact the responsible developer, providing the "
    "details shown by this error message."
;


/**
 */
static KiTVoid KI_CALL KiDbg_Internal_FormatMessage(KiSDebugTerminationContext const *termCtxt, KiTChar **bufPtr) {
    if (termCtxt == nullptr || bufPtr == nullptr) {
        if (bufPtr != nullptr)
            *bufPtr = nullptr;

        return;
    }

    /* Compute required size. */
    KiTSize reqSize = 0;
    {
        reqSize = (KiTSize)snprintf(
            nullptr,
            0,
            gl_c_ErrorMsgFormat,
            termCtxt->m_failedExpr.mp_strPtr,
            termCtxt->mp_errStr->mp_strPtr,
            termCtxt->m_errorCode,
            termCtxt->mp_errBrief->mp_strPtr,
            termCtxt->m_filePath.mp_strPtr,
            termCtxt->m_fileLine,
            termCtxt->m_fnName.mp_strPtr,
            termCtxt->mp_errDetails->mp_strPtr
        );

        /* Append terminating 'NUL'. */
        ++reqSize;
    }

    /* Allocate buffer. */
    *bufPtr = nullptr;
    {
        *bufPtr = calloc(1, reqSize * sizeof **bufPtr);

        if (*bufPtr == nullptr)
            return;
    }

    /* Format the error message so it looks good in a message box and acceptable in a terminal. */
    snprintf(
        *bufPtr,
        reqSize,
        gl_c_ErrorMsgFormat,
        termCtxt->m_failedExpr.mp_strPtr,
        termCtxt->mp_errStr->mp_strPtr,
        termCtxt->m_errorCode,
        termCtxt->mp_errBrief->mp_strPtr,
        termCtxt->m_filePath.mp_strPtr,
        termCtxt->m_fileLine,
        termCtxt->m_fnName.mp_strPtr,
        termCtxt->mp_errDetails->mp_strPtr
    );
}
/** \endcond */


KI_NORETURN KiTVoid KI_CALL KiDbgTerminateProcess(KiSDebugTerminationContext const *termCtxt) {
    static _Atomic KiTBool gl_actFlag;

    if (atomic_fetch_or(&gl_actFlag, KI_TRUE) == KI_FALSE) {
        /* Format the error message. */
        KiTChar *msgBufPtr;
        KiDbg_Internal_FormatMessage(termCtxt, &msgBufPtr);
        {
            /* Print error message or show message box or call the Oracle of Delphi for advice. */
            KiPlatform_DbgNotify(
                msgBufPtr != nullptr
                    ? msgBufPtr
                    : "****\n\nThere was a problem formatting the message buffer. \n\n****",
                termCtxt
            );
        }
        free(msgBufPtr);

        /* Actually exit. This does not return. */
        KiPlatform_DbgExit(termCtxt->m_errorCode);
    }

    /*
     * All other threads that might have called this function in parallel must not exit or return but must spin and wait
     * for the thread that actually got into the exit branch to terminate them. We cannot simply return and continue in
     * running the threads as normal as this function is not supposed to return. To reduce the CPU load, we just sleep
     * for 100ms. This might be necessary because KiPlatform_Notify() can block on some platforms due to them showing
     * interactive objects like message boxes, etc. The user can interact with these objects in any way they see fit,
     * and this can take an arbitrary amount of time.
     */
    while (KI_TRUE)
        KiPlatform_Sleep(100);
}


