/*****************************************************************************************************************
 * Kira - cross-platform 2-D role-playing game (RPG) game engine for desktop and mobile, and console platforms   *
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
#include <kira/dbg.h>

#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
static KiTChar const **gl_c_ErrorMsgFormat = &(KiTChar const *){
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
};


/**
 */
static KiTChar *KI_CALL Internal_FormatMessage(KiSDebugTerminationContext const *termCtxt) {
    KI_ASSERT(termCtxt != nullptr, KiErr_InParameter);

    /* Query error stringifications. */
    KiSStringView const *codeStr  = KiQueryErrorString(termCtxt->m_errorCode);
    KiSStringView const *briefStr = KiQueryErrorBrief(termCtxt->m_errorCode);
    KiSStringView const *detStr   = KiQueryErrorDetails(termCtxt->m_errorCode);

    /* Compute required size. */
    KiTSize reqSize = 0;
    {
        reqSize = (KiTSize)snprintf(
            nullptr,
            0,
            *gl_c_ErrorMsgFormat,
            termCtxt->m_failedExpr.mp_strPtr,
            codeStr->mp_strPtr,
            termCtxt->m_errorCode,
            briefStr->mp_strPtr,
            termCtxt->m_filePath.mp_strPtr,
            termCtxt->m_fileLine,
            termCtxt->m_fnName.mp_strPtr,
            detStr->mp_strPtr
        );

        /* Append terminating 'NUL'. */
        ++reqSize;
    }

    KiTChar *bufPtr = nullptr;
    {
        if ((bufPtr = calloc(1, reqSize * sizeof *bufPtr)) == nullptr)
            return nullptr;

        snprintf(
            bufPtr,
            reqSize,
            *gl_c_ErrorMsgFormat,
            termCtxt->m_failedExpr.mp_strPtr,
            codeStr->mp_strPtr,
            termCtxt->m_errorCode,
            briefStr->mp_strPtr,
            termCtxt->m_filePath.mp_strPtr,
            termCtxt->m_fileLine,
            termCtxt->m_fnName.mp_strPtr,
            detStr->mp_strPtr
        );
    }

    return bufPtr;
}
/** \endcond */


KI_NORETURN KiTVoid KI_CALL KiDebugTerminateProcess(KiSDebugTerminationContext const *termCtxt) {
    KI_ASSERT(termCtxt != nullptr, KiErr_InParameter);

    /** \cond */
    static _Atomic(KiTInt32) gl_actFlag;
    /** \endcond */

    if (atomic_fetch_or(&gl_actFlag, KI_TRUE) == KI_FALSE) {
        /* Format the error message. */
        KiTChar *msgBufPtr = Internal_FormatMessage(termCtxt);
        {
            /* Print error message or show message box or call the Oracle of Delphi for advice. */
            KiPlatform_Notify(
                msgBufPtr != nullptr
                    ? msgBufPtr
                    : "****\n\nThere was a problem formatting the message buffer. \n\n****",
                termCtxt
            );
        }
        free(msgBufPtr);

        /* Actually exit. This call does not return. */
        KiPlatform_Exit(termCtxt->m_errorCode);
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


