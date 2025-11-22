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
 * \file  krnlboot.h
 * \brief defines the actual kernel boot routines called by the kernel main in order to initialize all kernel-level
 *        (sub-)systems
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlStartSystems(KiTVoid);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlShutdownSystems(KiTVoid);


