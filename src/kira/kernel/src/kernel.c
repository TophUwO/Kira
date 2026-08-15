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
 * \file  kernel.c
 * \brief implements the kernel startup- and shutdown routines
 */


/* stdlib includes */
#if ((defined KI_PLATFORM_WINDOWS) && (!defined _CRT_SECURE_NO_WARNINGS))
    #define _CRT_SECURE_NO_WARNINGS 1
#endif
#include <stdatomic.h>

#include <string.h>

/* Kira includes */
#include <kira/kernel/rt.h>
#include <kira/kernel/json.h>

#include <kira/kernel/int/string.h>
#include <kira/kernel/int/platform.h>
#include <kira/kernel/int/fenum.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiStartKernelModules(KiTVoid);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiShutdownKernelModules(KiTVoid);


/**
 */
KI_NATIVE typedef struct KiSKernelState {
    KiSReturnState          m_retState;
    KiSRuntimeSpecification m_rtSpecs;
} KiSKernelState;

/**
 */
static KiSKernelState gl_KernelState = { 0 };
/** \endcond */


KiEErrorCode KI_CALL KiStartup(KiSRuntimeSpecification const *rtSpecs) {
    if (rtSpecs == nullptr)
        return KiErr_InParameter;

    /* Clear kernel state. This is to be done in case of a restart via KiKrnlRestart(). */
    memset(&gl_KernelState, 0, sizeof gl_KernelState);
    /* Copy runtime specification. */
    KiTSize size2Copy = KI_MIN(sizeof gl_KernelState.m_rtSpecs, rtSpecs->m_structSize);
    {
        memcpy((KiTVoid *)&gl_KernelState.m_rtSpecs, (KiTVoid const *)rtSpecs, size2Copy);

        gl_KernelState.m_rtSpecs.m_structSize = size2Copy;
    }

    /* Load kernel modules. This will also load a configuration and profile. */
    KiEErrorCode errCode = KiStartKernelModules();
    if (errCode != KiErr_Ok)
        return errCode;

    // enum all modules
    // add all comps to reg
    // load the rest of the modules (via iterative dfs)

    /* All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiShutdown(KiTVoid) {
    KiEErrorCode const errCode = KiShutdownKernelModules();

    return errCode;
}

KiTVoid KI_CALL KiRun(KiSReturnState *retStatePtr) {
    /* Locate client and invoke Run(). */

    /* At last, copy runtime return state. */
    KiTSize const dstSize = retStatePtr->m_structSize;
    {
        memcpy(retStatePtr, &gl_KernelState.m_retState, gl_KernelState.m_retState.m_structSize);

        retStatePtr->m_structSize = dstSize;
    }
}

KiTVoid KI_CALL KiQuit(KiEErrorCode errCode) {
    KiSReturnState *const krnlRetState = &gl_KernelState.m_retState;

    *krnlRetState = (KiSReturnState){
        .m_structSize   = sizeof *krnlRetState,
        .m_errCode      = errCode,
        .m_wantsRestart = KI_FALSE
    };
}

KiTVoid KI_CALL KiRestart(KiEErrorCode errCode) {
    KiSReturnState *const krnlRetState = &gl_KernelState.m_retState;

    *krnlRetState = (KiSReturnState){
        .m_structSize   = sizeof *krnlRetState,
        .m_errCode      = errCode,
        .m_wantsRestart = KI_TRUE
    };
}


KiSReturnState *KI_CALL KiGetReturnState(KiTVoid) {
    return (KiSReturnState *)&gl_KernelState.m_retState;
}

KiSRuntimeSpecification const *KI_CALL KiGetRuntimeSpecification(KiTVoid) {
    return (KiSRuntimeSpecification const *)&gl_KernelState.m_rtSpecs;
}


KiTVoid KI_CALL KiInvokeOnAssertHandler(KiTVoid const *extraParam) {
    
}


