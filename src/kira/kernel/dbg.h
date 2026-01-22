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
 * \file  dbg.h
 * \brief defines global services used for debugging purposes
 */


#pragma once

/* Kira includes */
#include <kira/dbg.h>


/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiStartDebugSession(KiSDebugOptions const *dbgOpt);
/** 
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiStopDebugSession(KiTVoid);

/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiDebugTerminateProcess(KiSDebugTerminationContext const *termCtxt);

/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiRegisterDebugEventHandler(KiIDebugEventHandler *dbgModuleRef);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiUnregisterDebugEventHandler(KiIDebugEventHandler *dbgModuleRef);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiFlushDebugEvents(KiTVoid);


