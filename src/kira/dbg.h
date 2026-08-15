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
 * \file  dbg.h
 * \brief defines the public API for the Kira Debugging System (KiDbg)
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


#if (defined KI_CONFIG_DEBUG)
    /* Get the function signature; non-standard. */
    #if (defined _MSC_VER)
        /**
         */
        #define __KIRA_FUNCSIG__ "__FUNCSIG__"
    #else
        #define __KIRA_FUNCSIG__ __PRETTY_FUNCTION__
    #endif /* (defined _MSC_VER) */

    /**
     */
    #define KI_ASSERT(expr, errorCode)                                  \
        if (!(expr)) {                                                  \
            KiAbort(&(KiSAbortContext const){                           \
                .m_structSize  = sizeof(KiSAbortContext),               \
                .m_failedExpr  = KI_MAKE_STRING_VIEW(#expr),            \
                .m_filePath    = KI_MAKE_STRING_VIEW(__FILE__),         \
                .m_fileLine    = (KiTUint64)__LINE__,                   \
                .m_fnName      = KI_MAKE_STRING_VIEW(__func__),         \
                .m_fnSig       = KI_MAKE_STRING_VIEW(__KIRA_FUNCSIG__), \
                .m_errorCode   = (enum KiEErrorCode)(errorCode),        \
                .m_extraMsg    = KI_MAKE_STRING_VIEW("")                \
            });                                                         \
        }
#else
    #define __KIRA_FUNCSIG__
    #define KI_ASSERT(expr, errorCode)
#endif /* (defined KI_CONFIG_DEBUG) */


/**
 */
KI_NATIVE typedef struct KiSAbortContext {
    KiTSize       m_structSize;
    KiSStringView m_failedExpr;
    KiSStringView m_filePath;
    KiTUint64     m_fileLine;
    KiSStringView m_fnName;
    KiSStringView m_fnSig;
    KiEErrorCode  m_errorCode;
    KiSStringView m_extraMsg;
} KiSAbortContext;


/**
 */
KI_NATIVE KI_API KI_NORETURN KiTVoid KI_CALL KiAbort(KiSAbortContext const *abortCtxtPtr);
