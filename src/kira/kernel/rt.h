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
 * \file  rt.h
 * \brief defines the public Kira kernel APIs dedicated for runtime control
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>
#include <kira/dbg.h>
#include <kira/app.h>

#include <kira/kernel/json.h>


/**
 */
KI_NATIVE typedef struct KiSRuntimeSpecification {
    KiTSize   m_structSize;
    int       m_argc;
    char    **mpp_argv;
    char    **mpp_envp;
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
KI_NATIVE KI_API KiSReturnState *KI_CALL KiGetReturnState(KiTVoid);
/**
 */
KI_NATIVE KI_API KiSRuntimeSpecification const *KI_CALL KiGetRuntimeSpecification(KiTVoid);
/**
 */
KI_NATIVE KI_API KiSJson const *KI_CALL KiGetRuntimeConfiguration(KiTVoid);
