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
 * \file  rt.h
 * \brief defines the public Kira kernel APIs dedicated for runtime control
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>
#include <kira/dbg.h>

#include <kira/kernel/json.h>


/**
 */
KI_NATIVE typedef enum KiERuntimeMode : KiTUint32 {
    KiRtMode_None = 0,

    KiRtMode_Wait = 1,
    KiRtMode_Poll = 2,

    __KiRtMode_Count__
} KiERuntimeMode;


/**
 */
KI_NATIVE typedef struct KiSReturnState {
    KiTSize      m_structSize;
    KiEErrorCode m_errCode;
    KiTBool      m_wantsRestart;
} KiSReturnState;

/**
 */
KI_NATIVE typedef struct KiSRuntimeSpecification {
    KiTSize           m_structSize;
    int               m_argc;
    char            **mpp_argv;
    char            **mpp_envp;
    KiSDebugOptions  *mp_dbgOpts;
    KiERuntimeMode    m_rtMode;
} KiSRuntimeSpecification;


/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiStartup(KiSRuntimeSpecification const *rtSpecs);
/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiShutdown(KiTVoid);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiRun(KiSReturnState *retStatePtr);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiQuit(KiEErrorCode errCode);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiRestart(KiEErrorCode errCode);

/**
 */
KI_NATIVE KI_API KiSRuntimeSpecification const *KI_CALL KiGetRuntimeSpecification(KiTVoid);
/**
 */
KI_NATIVE KI_API KiSJson const *KI_CALL KiGetRuntimeConfig(KiTVoid);

/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiInvokeOnAssertHandler(KiTVoid const *extraParam);

/**
 */
KI_NATIVE KI_API KiTInt8 KI_CALL KiCompareVersions(KiSVersion const *lVerPtr, KiSVersion const *rVerPtr);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiIsVersionInRange(
    KiSVersion const *verPtr,
    KiSVersion const *minVerPtr,
    KiSVersion const *maxVerPtr,
    KiTBool isInclusive
);

