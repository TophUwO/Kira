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
#include <kira/kcm.h>


/**
 */
KI_NATIVE KI_API KiTChar const *KI_CALL KiKrnlGetApplicationRootDirectory(KiTVoid);
/**
 */
KI_NATIVE KI_API KiTChar const *KI_CALL KiKrnlGetWorkingDirectory(KiTVoid);
/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiKrnlSetWorkingDirectory(KiTChar const *newWorkingDir);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiKrnlNativeSeparatorsToKiraSeparators(KiTChar *pathStr);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiKrnlSplitPath(
    KiTChar const *fullPathStr,
    KiSStringView *dirViewPtr, 
    KiSStringView *dirNameViewPtr,
    KiSStringView *fileNameViewPtr,
    KiSStringView *fileExtViewPtr
);


