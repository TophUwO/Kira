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
 * \file  error.c
 * \brief implements the error stringification service
 */


/* stdlib includes */
#include <string.h>
#include <errno.h>

/* Kira includes */
#include <kira/util.h>
#include <kira/dbg.h>

#include <kira/kernel/reg.h>

#include <kira/kernel/int/array.h>
#include <kira/kernel/int/sync.h>
#include <kira/kernel/int/krnlmod.h>


/** \cond INTERNAL */
/**
 */
#define KI_KRNLERR_TOLOCAL(x) (((KiEErrorCode)(x)) - KiErr_Ok)


/**
 */
KI_NATIVE typedef struct KiSErrorStringificationContext {
    KiTUint16            m_modId;
    KiTUint16            m_locErrCode;
    KiIErrorStringifier *mp_errStrRef;
} KiSErrorStringificationContext;

/**
 * \struct KiSErrorStrDirectory
 * \brief  represents the error stringifier directory
 */
KI_NATIVE typedef struct KiSErrorStringificationServiceState {
    KiTBool              m_isInit;     /**< init flag for module */
    KiTRWLockHandle      m_rwLock;     /**< reader/writer lock for slot vector */
    KiSArray            *mp_slotVec;   /**< error stringifier storage */
    KiIErrorStringifier *mp_defErrStr; /**< error stringifier for the default error codes */
} KiSErrorStringificationServiceState;

/**
 */
static KiSErrorStringificationServiceState gl_ErrorStringificationSrv;
/**
 * \brief actual instance of the default and always available error stringifier
 */
static KiIErrorStringifier gl_c_DefaultErrorStringifier;


/**
 */
static KiEErrorCode KI_CALL KiInternal_DetermineFinalStringificationContext(
    KiEErrorCode errCode,
    KiSErrorStringificationContext *strCtxtPtr,
    KiSStringView const *(KI_CALL *fnTest)(KiIErrorStringifier *, KiEErrorCode)
) {
    KI_ASSERT(strCtxtPtr != nullptr, KiErr_InOutParameter);

    /* Parse global error code. */
    *strCtxtPtr = (KiSErrorStringificationContext){
        .m_modId      = errCode >> 16,
        .m_locErrCode = errCode & 0xFFFF,
        .mp_errStrRef = nullptr
    };
    KiEErrorCode errorCode = KiErr_Ok;

    /* Get error stringifier. */
    KiAcquireRead(&gl_ErrorStringificationSrv.m_rwLock);
    {
        strCtxtPtr->mp_errStrRef = KiGetArrayElementAt(gl_ErrorStringificationSrv.mp_slotVec, strCtxtPtr->m_modId);

        /*
         * If there is no stringifier registered for the module which the error code came from, pick the default error
         * stringifier.
         * Because the module which this code came from is unknown, the error code is also likely unknown.
         * Therefore, we set it to 'KiErr_Unknown' to not accidentally return info for a valid default error code
         * since local error codes are generally in the same domain (0x0000...0xFFFF).
         * Also invoke the test function. If it returns nullptr, then the stringifier cannot determine a valid
         * stringification. In this case, also pick the default stringifier alongside 'KiErr_Unknown'.
         */
        if (strCtxtPtr->mp_errStrRef == nullptr || (*fnTest)(strCtxtPtr->mp_errStrRef, strCtxtPtr->m_locErrCode)) {
            errorCode = strCtxtPtr->mp_errStrRef == nullptr ? KiErr_UnknownModule : KiErr_UnknownErrorCode;

            *strCtxtPtr = (KiSErrorStringificationContext){
                .m_modId      = strCtxtPtr->m_modId,
                .m_locErrCode = KiErr_Unknown & 0xFFFF,
                .mp_errStrRef = gl_ErrorStringificationSrv.mp_defErrStr
            };
        }
    }
    KiReleaseRead(&gl_ErrorStringificationSrv.m_rwLock);

    return errCode;
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(ErrorStringificationService)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /* Create slot vector. */
    KiSArray *slVec;
    {
        KiEErrorCode errorCode = KiCreateArray(&slVec);

        if (errorCode != KiErr_Ok)
            return errorCode;
    }

    /* Create RW lock. */
    KiTRWLockHandle rwLock;
    {
        KiEErrorCode errorCode = KiCreateRWLock(&rwLock);

        if (errorCode != KiErr_Ok) {
            KiDestroyArray(slVec);

            return errorCode;
        }
    }

    /* Initialize global state. */
    gl_ErrorStringificationSrv = (KiSErrorStringificationServiceState){
        .m_rwLock     = rwLock,
        .mp_slotVec   = slVec,
        .mp_defErrStr = &gl_c_DefaultErrorStringifier
    };
    gl_c_DefaultErrorStringifier.VT->Acquire(&gl_c_DefaultErrorStringifier);

    /* All good. */
    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(ErrorStringificationService)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    KiDestroyRWLock(&gl_ErrorStringificationSrv.m_rwLock);
    KiDestroyArray(gl_ErrorStringificationSrv.mp_slotVec);

    gl_c_DefaultErrorStringifier.VT->Release(&gl_c_DefaultErrorStringifier);
    return KiErr_Ok;
}
/** \endcond */


KiEErrorCode KI_CALL KiRegisterErrorStringifier(KiIErrorStringifier *errStrRef, KiTUint32 *offsetPtr) {
    KI_ASSERT(errStrRef != nullptr, KiErr_KiOMComponentParameter);
    KI_ASSERT(offsetPtr != nullptr, KiErr_OutParameter);

    KiTIndex actOffset;
    KiEErrorCode errCode;
    {
        /* Insert and record index. */
        KiAcquireWrite(&gl_ErrorStringificationSrv.m_rwLock);
        {
            errCode = KiInsertIntoArray(gl_ErrorStringificationSrv.mp_slotVec, errStrRef, &actOffset);
        }
        KiReleaseWrite(&gl_ErrorStringificationSrv.m_rwLock);

        /* Insertion failed. */
        if (errCode != KiErr_Ok)
            return errCode;
        /* Insertion succeeded. Add reference to stringifier. */
        errStrRef->VT->Acquire(errStrRef);
    }
    *offsetPtr = (KiTUint32)actOffset;

    /* All good. */
    return KiErr_Ok;
}

KiTVoid KI_CALL KiKrnlUnregisterErrorStringifier(KiTUint32 offset) {
    /* Erase from slot vector. */
    KiAcquireWrite(&gl_ErrorStringificationSrv.m_rwLock);
    {
        KiIErrorStringifier *errStrRef = KiEraseFromArray(gl_ErrorStringificationSrv.mp_slotVec, (KiTIndex)offset);

        /* Valid offset. Release. */
        if (errStrRef != nullptr)
            errStrRef->VT->Release(errStrRef);
    }
    KiReleaseWrite(&gl_ErrorStringificationSrv.m_rwLock);
}

KiSStringView const *KI_CALL KiQueryErrorString(KiEErrorCode errorCode) {
    /*
     * Get stringification context from error code. This gives us the module ID, the local error code and the
     * stringifier that can properly stringifier the given error code.
     */
    KiSErrorStringificationContext strCtxt;
    {
        KiIErrorStringifier *defErrStr = gl_ErrorStringificationSrv.mp_defErrStr;

        KiInternal_DetermineFinalStringificationContext(errorCode, &strCtxt, defErrStr->VT->QueryErrorString);
    }

    /* Get final result value. */
    return strCtxt.mp_errStrRef->VT->QueryErrorString(strCtxt.mp_errStrRef, strCtxt.m_locErrCode);
}

KiSStringView const *KI_CALL KiQueryErrorBrief(KiEErrorCode errorCode) {
    /*
     * Get stringification context from error code. This gives us the module ID, the local error code and the
     * stringifier that can properly stringifier the given error code.
     */
    KiSErrorStringificationContext strCtxt;
    {
        KiIErrorStringifier *defErrStr = gl_ErrorStringificationSrv.mp_defErrStr;

        KiInternal_DetermineFinalStringificationContext(errorCode, &strCtxt, defErrStr->VT->QueryErrorBrief);
    }

    /* Get final result value. */
    return strCtxt.mp_errStrRef->VT->QueryErrorBrief(strCtxt.mp_errStrRef, strCtxt.m_locErrCode);
}

KiSStringView const *KI_CALL KiQueryErrorDetails(KiEErrorCode errorCode) {
    /*
     * Get stringification context from error code. This gives us the module ID, the local error code and the
     * stringifier that can properly stringifier the given error code.
     */
    KiSErrorStringificationContext strCtxt;
    {
        KiIErrorStringifier *defErrStr = gl_ErrorStringificationSrv.mp_defErrStr;

        KiInternal_DetermineFinalStringificationContext(errorCode, &strCtxt, defErrStr->VT->QueryErrorDetails);
    }

    /* Get final result value. */
    return strCtxt.mp_errStrRef->VT->QueryErrorDetails(strCtxt.mp_errStrRef, strCtxt.m_locErrCode);
}


/* Default error stringifier implementation. */
#pragma region Default Error Stringifier
/**
 * \brief dictionary holding the stringifications for the numeric error code identifiers themselves
 */
static KiSStringView const *gl_c_ErrorStringDictionary[] = {
    &KI_MAKE_STRING_VIEW("KiErr_Ok"),
    &KI_MAKE_STRING_VIEW("KiErr_Unknown"),
    &KI_MAKE_STRING_VIEW("KiErr_NoOperation"),
    &KI_MAKE_STRING_VIEW("KiErr_ManuallyAborted"),
    &KI_MAKE_STRING_VIEW("KiErr_NotImplemented"),
    &KI_MAKE_STRING_VIEW("KiErr_InParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_OutParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_InOutParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_CallbackParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_InptrParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_OutptrParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_SelfParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_KiOMComponentParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_EnumParameter"),
    &KI_MAKE_STRING_VIEW("KiErr_NoInstance"),
    &KI_MAKE_STRING_VIEW("KiErr_NoMetaClasses"),
    &KI_MAKE_STRING_VIEW("KiErr_InterfaceNotImplemented"),
    &KI_MAKE_STRING_VIEW("KiErr_MemoryAllocation"),
    &KI_MAKE_STRING_VIEW("KiErr_MemoryReallocation"),
    &KI_MAKE_STRING_VIEW("KiErr_CapLimitExceeded"),
    &KI_MAKE_STRING_VIEW("KiErr_ContainerFull"),
    &KI_MAKE_STRING_VIEW("KiErr_ItemNotFound"),
    &KI_MAKE_STRING_VIEW("KiErr_ItemAlreadyExists"),
    &KI_MAKE_STRING_VIEW("KiErr_SynchInit"),
    &KI_MAKE_STRING_VIEW("KiErr_RangeError"),
    &KI_MAKE_STRING_VIEW("KiErr_IndexError"),
    &KI_MAKE_STRING_VIEW("KiErr_DlibLoadLibrary"),
    &KI_MAKE_STRING_VIEW("KiErr_DlibUnloadLibrary"),
    &KI_MAKE_STRING_VIEW("KiErr_DlibResolveSymbol"),
    &KI_MAKE_STRING_VIEW("KiErr_IncorrectInitState"),
    &KI_MAKE_STRING_VIEW("KiErr_UnsupportedUsageCtxt"),
    &KI_MAKE_STRING_VIEW("KiErr_IllegalObjectState"),
    &KI_MAKE_STRING_VIEW("KiErr_InvalidApi"),
    &KI_MAKE_STRING_VIEW("KiErr_UnknownModule"),
    &KI_MAKE_STRING_VIEW("KiErr_UnknownErrorCode"),
    &KI_MAKE_STRING_VIEW("KiErr_InvalidEnumContextType"),
    &KI_MAKE_STRING_VIEW("KiErr_NoSuchFileOrDirectory"),
    &KI_MAKE_STRING_VIEW("KiErr_InsufficientFilePerms"),
    &KI_MAKE_STRING_VIEW("KiErr_EntityIsDirectory"),
    &KI_MAKE_STRING_VIEW("KiErr_EntityIsNotADirectory"),
    &KI_MAKE_STRING_VIEW("KiErr_EncodingError"),

    [61] = &KI_MAKE_STRING_VIEW("")
};
KI_VERIFY_LUT(gl_c_ErrorStringDictionary, __KiErr_Count__ - KiErr_Ok);

/**
 * \brief dictionary mapping numeric error codes to a brief description of the error condition represented by the
 *        corresponding numeric error code
 */
static KiSStringView const *gl_c_ErrorBriefDictionary[] = {
    [61] = &KI_MAKE_STRING_VIEW("")
};
KI_VERIFY_LUT(gl_c_ErrorBriefDictionary, __KiErr_Count__ - KiErr_Ok);

/**
 * \brief dictionary with the detailed error code descriptions for each default error code
 */
static KiSStringView const *gl_c_ErrorDetailsDictionary[] = {
    &KI_MAKE_STRING_VIEW(
        "This \"error\" code means that no actual error actually happened. It serves as a universal return code "
        "that signifies successful completion of the returning function. However, never assume this error code "
        "corresponds to numeric zero as default error codes are global by default."
    ),
    &KI_MAKE_STRING_VIEW(
        "\"KiErr_Unknown\" has many different meanings: It's used for when an unexpected error occurred, for when "
        "the error stringifier could not successfully stringify the numeric error code (e.g., due to it being "
        "unknown), for when no error stringifier is available for the module from which the (global) error code "
        "originated from, and many more."
    ),
    &KI_MAKE_STRING_VIEW(
        "This error code does not signify an error but indicates that the function did nothing. It is returned "
        "when a function determines that normal execution is not needed but could be done regardless."
    ),
    &KI_MAKE_STRING_VIEW(
        "This return code does not indicate an error. It is used by callbacks that are invoked in loops to "
        "indicate that iteration be terminated prematurely. It is useful for searching an array for the first "
        "occurrence of an element, validation, etc."
    ),
    &KI_MAKE_STRING_VIEW(
        "This return code means that the callee (or a callee of the callee) (i.e., a function being called) is not "
        "(yet) implemented. It can be returned during development and can be used as the return value for default "
        "implementations for interface methods. In the deployed code, this code should never be returned."
    ),
    [61] = &KI_MAKE_STRING_VIEW("")
};
KI_VERIFY_LUT(gl_c_ErrorDetailsDictionary, __KiErr_Count__ - KiErr_Ok);


/**
 */
static KiTInt32 KI_CALL KiCDefaultErrorStringifier_KiIErrorStringifier_Acquire(KiIErrorStringifier *self) {
    KI_UNREFERENCED_PARAMETER(self);
    KI_ASSERT(self != nullptr, KiErr_SelfParameter);

    /* Stub. */
    return 1;
}

/**
 */
static KiTInt32 KI_CALL KiCDefaultErrorStringifier_KiIErrorStringifier_Release(KiIErrorStringifier *self) {
    KI_UNREFERENCED_PARAMETER(self);
    KI_ASSERT(self != nullptr, KiErr_SelfParameter);
    
    /* Stub. */
    return 1;
}

/**
 */
static KiEErrorCode KI_CALL KiCDefaultErrorStringifier_KiIErrorStringifier_QueryInterface(
    KiIErrorStringifier *self,
    KiTChar const *iId,
    KiTVoid **resPtr
) {
    KI_ASSERT(self != nullptr,   KiErr_SelfParameter);
    KI_ASSERT(iId != nullptr,    KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_KiOMComponentParameter);

    if (!strcmp(iId, "KiIBase") || !strcmp(iId, "KiIErrorStringifier")) {
        /* Interface is implemented. */
        *resPtr = (KiTVoid *)self;

        KiCDefaultErrorStringifier_KiIErrorStringifier_Acquire(self);
        return KiErr_Ok;
    }

    /* Interface not implemented. */
    *resPtr = nullptr;
    return KiErr_InterfaceNotImplemented;
}

/**
 */
static KiSStringView const *KI_CALL KiCDefaultErrorStringifier_KiIErrorStringifier_QueryErrorString(
    KiIErrorStringifier *self,
    KiEErrorCode errorCode
) {
    KI_ASSERT(self != nullptr, KiErr_SelfParameter);

    /* Query the stringification. If it's out of range, return the stringification for 'KiErr_Unknown'. */
    return (KiSStringView const *)(errorCode < KI_COUNTOF(gl_c_ErrorStringDictionary)
        ? gl_c_ErrorStringDictionary[errorCode]
        : self->VT->QueryErrorString(self, KiErr_Unknown & 0xFFFF)
    );
}

/**
 */
static KiSStringView const *KI_CALL KiCDefaultErrorStringifier_KiIErrorStringifier_QueryErrorBrief(
    KiIErrorStringifier *self,
    KiEErrorCode errorCode
) {
    KI_ASSERT(self != nullptr, KiErr_SelfParameter);

    /* Query the stringification. If it's out of range, return the stringification for 'KiErr_Unknown'. */
    return (KiSStringView const *)(errorCode < KI_COUNTOF(gl_c_ErrorBriefDictionary)
        ? gl_c_ErrorBriefDictionary[errorCode]
        : self->VT->QueryErrorBrief(self, KiErr_Unknown & 0xFFFF)
    );
}

/**
 */
static KiSStringView const *KI_CALL KiCDefaultErrorStringifier_KiIErrorStringifier_QueryErrorDetails(
    KiIErrorStringifier *self,
    KiEErrorCode errorCode
) {
    KI_ASSERT(self != nullptr, KiErr_SelfParameter);

    /* Query the stringification. If it's out of range, return the stringification for 'KiErr_Unknown'. */
    return (KiSStringView const *)(errorCode < KI_COUNTOF(gl_c_ErrorDetailsDictionary)
        ? gl_c_ErrorDetailsDictionary[errorCode]
        : self->VT->QueryErrorDetails(self, KiErr_Unknown & 0xFFFF)
    );
}


/** \cond */
static KiIErrorStringifier gl_c_DefaultErrorStringifier = {
    .VT = &(struct __KiIErrorStringifier_VTable__ const){
        .QueryInterface    = &KiCDefaultErrorStringifier_KiIErrorStringifier_QueryInterface,
        .Acquire           = &KiCDefaultErrorStringifier_KiIErrorStringifier_Acquire,
        .Release           = &KiCDefaultErrorStringifier_KiIErrorStringifier_Release,
        .QueryErrorString  = &KiCDefaultErrorStringifier_KiIErrorStringifier_QueryErrorString,
        .QueryErrorBrief   = &KiCDefaultErrorStringifier_KiIErrorStringifier_QueryErrorBrief,
        .QueryErrorDetails = &KiCDefaultErrorStringifier_KiIErrorStringifier_QueryErrorDetails
    }
};
/** \endcond */
#pragma endregion


KiEErrorCode KI_CALL KiNativeErrorCodeToKiraErrorCode(KiTInt32 ntErrorCode) {
    /// TODO: implement actual native error code translation
    return KiErr_NotImplemented;
}

KiEErrorCode KI_CALL KiErrnoToKiraErrorCode(KiTInt32 errnoCode) {
    /**
     * \brief maps a native (i.e., \c errno) error code to a corresponding Kira error code
     */
    static constexpr KiEErrorCode gl_c_Errno2KiraErrorCodeDirectory[] = {
        [EACCES]  = KiErr_InsufficientFilePerms,
        [EISDIR]  = KiErr_EntityIsADirectory,
        [ENOENT]  = KiErr_NoSuchFileOrDirectory,
        [ENOMEM]  = KiErr_MemoryAllocation,
        [ENOTDIR] = KiErr_EntityIsNotADirectory
    };
    /** \cond */
    static constexpr KiTSize gl_c_DictSize = KI_COUNTOF(gl_c_Errno2KiraErrorCodeDirectory);
    /** \endcond */

    /*
     * If the error code is out of bounds or is not assigned, return the numeric identifier for Kira's 'unknown' error
     * code.
     */
    return errnoCode < 0 || errnoCode >= (KiTInt32)gl_c_DictSize || gl_c_Errno2KiraErrorCodeDirectory[errnoCode] == 0
        ? KiErr_Unknown
        : gl_c_Errno2KiraErrorCodeDirectory[errnoCode]
    ;
}


/** \cond */
KI_KRNLMOD_DEFINE(ErrorStringificationService) {
    .m_structSize = sizeof(KiSModuleInfo),
    .mp_modUuid   = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId     = &KI_MAKE_STRING_VIEW("error stringification service"),
    .m_modFlags   = 0,

    .mp_fnInit    = &KI_KRNLMOD_INITFN(ErrorStringificationService),
    .mp_fnUninit  = &KI_KRNLMOD_UNINITFN(ErrorStringificationService)
};
/** \endcond */
