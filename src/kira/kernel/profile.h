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
 * \file  profile.h
 * \brief defines the API for the kernel-level profile manager
 *
 * In Kira, \e profiles are files configuring the behavior of the application, especially the behavior of the
 * dependency injection mechanism.
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/json.h>


/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiLoadProfile(KiTChar const *profilePath);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiUnloadProfile(KiTVoid);
/**
 */
KI_NATIVE KI_API KiSJson const *KI_CALL KiGetProfile(KiTVoid);
/**
 */
KI_NATIVE KI_API KiTChar const *KI_CALL KiGetCurrentProfilePath(KiTVoid);
/**
 */
KI_NATIVE KI_API KiTChar const *KI_CALL KiGetProfileDirectoryPath(KiTVoid);


