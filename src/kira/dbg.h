/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                  *
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
    /**
     */
    #define KI_ASSERT(expr, errorCode)                                           \
        if (!(expr)) {                                                           \
            KiSDebugOptions const *dbgOpt = KiGetDebugOptions();                 \
            if (!dbgOpt->m_isAssertsEnabled)                                     \
                goto KI_CONCAT2(__lbl_END_, __LINE__);                           \
                                                                                 \
            KiRaiseException(&(KiSException const){                              \
                .m_structSize  = sizeof(KiSException),                           \
                .m_excType     = KiExcTy_AssertionFailed,                        \
                .mp_excDataPtr = (KiTVoid *)&(KiSDebugTerminationContext const){ \
                    .m_structSize  = sizeof(KiSDebugTerminationContext),         \
                    .m_failedExpr  = KI_MAKE_STRING_VIEW(#expr),                 \
                    .m_filePath    = KI_MAKE_STRING_VIEW(__FILE__),              \
                    .m_fileLine    = (KiTUint64)__LINE__,                        \
                    .m_fnName      = KI_MAKE_STRING_VIEW(__func__),              \
                    .m_fnSig       = KI_MAKE_STRING_VIEW(__func__),              \
                    .m_errorCode   = (enum KiEErrorCode)(errorCode),             \
                    .m_extraMsg    = KI_MAKE_STRING_VIEW("")                     \
                },                                                               \
                .m_excDataSize = sizeof(KiSDebugTerminationContext)              \
            }, nullptr);                                                         \
            KI_CONCAT2(__lbl_END_, __LINE__):                                    \
            (KiTVoid)(0);                                                        \
        }
#else
    #define KI_ASSERT(expr, errorCode)
#endif


/** \cond */
KI_NATIVE typedef struct KiSException KiSException;
/** \endcond */

/**
 */
KI_NATIVE typedef KiTUint64 KiTDebugEventType;

/**
 */
KI_NATIVE typedef KiTVoid (KI_CALL *KiFExceptionHandler)(KiSException const *excPtr, KiTVoid *extraParam);


/**
 */
KI_NATIVE typedef enum KiEExceptionType {
    KiExcTy_Invalid,

    KiExcTy_AssertionFailed,

    KI_ENUM_COUNT(KiExcTy)
} KiEExceptionType;


/**
 */
KI_NATIVE typedef struct KiSException {
    KiTSize           m_structSize;
    KiEExceptionType  m_excType;
    KiTVoid          *mp_excDataPtr;
    KiTSize           m_excDataSize;
} KiSException;

/**
 */
KI_NATIVE typedef struct KiSDebugOptions {
    KiTSize m_structSize;
    KiTBool m_useDetRng;
    KiTBool m_isAssertsEnabled;
    KiTBool m_registerOnAssertHandler;
} KiSDebugOptions;

/**
 */
KI_NATIVE typedef struct KiSDebugTerminationContext {
    KiTSize       m_structSize;
    KiSStringView m_failedExpr;
    KiSStringView m_filePath;
    KiTUint64     m_fileLine;
    KiSStringView m_fnName;
    KiSStringView m_fnSig;
    KiEErrorCode  m_errorCode;
    KiSStringView m_extraMsg;
} KiSDebugTerminationContext;

/**
 */
KI_NATIVE typedef struct KiSDebugEvent {
    KiTDebugEventType  m_evType;
    KiTFlags32         m_evFlags;
    KiTVoid           *mp_evData;
    KiTSize            m_dataSize;
} KiSDebugEvent;


/**
 * \interface KiIDebugEventHandler
 * \brief     handles debug events
 */
KI_INTERFACE(KiIDebugEventHandler) {
    KI_METADATA(
        "uuid":    "2d2b734f-10a5-473c-aace-cc59a01ffafe",
        "name":    "KiIDebugEventHandler",
        "dname":   "IDebugEventHandler",
        "sname":   "DBGEVHND",
        "brief":   "provides an API able to process debug events",
        "version": [1, 0, 0],
        "legal":   {
            "author":    "TophUwO",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2025 TophUwO"
        }
    )
    KI_BASE(KiIDebugEventHandler)

    /**
     */
    KiTVoid (KI_CALL *OnDebugEvent)(KiIDebugEventHandler *self, KiTDebugEventType evType, KiSDebugEvent const *evData);
};


/**
 */
KI_NATIVE KI_API KiSDebugOptions const *KiGetDebugOptions(KiTVoid);

/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiPostDebugEvent(KiSDebugEvent const *dbgEventPtr);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiRaiseException(KiSException const *excPtr, KiTVoid *extraParam);


