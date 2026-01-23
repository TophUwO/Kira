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
 * \file  winterm.c
 * \brief implements functions for the debug termination service specific to the Windows(R) platform
 */
#if (defined KI_PLATFORM_WINDOWS)


/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/kernel/dbg.h>

#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
KiTVoid KI_CALL KiPlatform_Notify(KiTChar const *mBuf, KiSDebugTerminationContext const *tCtxt) {
    WCHAR *tmpBuf = KiPlatform_CreateFromKiraEncoding(mBuf, KI_DONTCARE(KiTSize));

    MessageBoxW(nullptr, tmpBuf, u"Debug Error", MB_OK);

    free(tmpBuf);
}

KI_NORETURN KiTVoid KI_CALL KiPlatform_Exit(KiEErrorCode errCode) {
    ExitProcess((UINT)errCode);
}
/** \endcond */


#endif /* defined KI_PLATFORM_WINDOWS */


