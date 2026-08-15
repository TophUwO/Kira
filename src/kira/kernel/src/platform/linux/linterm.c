/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

/**
 * \file  linterm.c
 * \brief implements Linux-specific routine for process termination
 */


#if (defined KI_PLATFORM_LINUX)
/* stdlib includes */
#include <stdio.h>
#include <stdlib.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


KiTVoid KI_CALL KiPlatform_Notify(KiTChar const *mBuf, KiSDebugTerminationContext const *tCtxt) {
    KI_ASSERT(mBuf != nullptr,  KiErr_InParameter);
    KI_ASSERT(*mBuf != '\0',    KiErr_InParameter);
    KI_ASSERT(tCtxt != nullptr, KiErr_InParameter);

    KI_IGNORE_RETURN_VALUE(fputs((char const *)mBuf, stderr));
}

KiTVoid KI_CALL KiPlatform_Exit(KiEErrorCode errCode) {
    exit((int)errCode);
}
#endif /* (defined KI_PLATFORM_LINUX) */
