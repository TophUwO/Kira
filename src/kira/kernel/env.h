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
 * \file  dir.h
 * \brief defines the public API for application directory control
 */


#pragma once

/* Kira includes */
#include <kira/def.h>
#include <kira/util.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiKrnlGetCommandLineArgument(KiTChar const *keyStr, KiSStringView *dstPtr);
/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiKrnlGetEnvironmentVariable(KiTChar const *varName, KiSStringView *dstPtr);


