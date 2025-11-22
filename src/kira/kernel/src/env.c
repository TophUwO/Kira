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
 * \file  env.c
 * \brief implements the API functions for the environment variables
 */


/* Kira includes */
#include <kira/error.h>

#include <kira/kernel/env.h>
#include <kira/kernel/reg.h>

#include <kira/dbg/dbg.h>


KiEErrorCode KI_CALL KiKrnlGetCommandLineArgument(KiTChar const *keyStr, KiSStringView *dstPtr) {
    KI_ASSERT(keyStr != nullptr, KiErr_InParameter);
    KI_ASSERT(*keyStr != '\0',   KiErr_InParameter);
    KI_ASSERT(dstPtr != nullptr, KiErr_OutParameter);

    /* All good. */
    return KiErr_Ok;
}


