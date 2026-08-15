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
 * \file  winterm.c
 * \brief implements functions for the debug termination service specific to the Windows(R) platform
 */


#if (defined KI_PLATFORM_WINDOWS)
/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
KiTVoid KI_CALL KiPlatform_Notify(KiTChar const *mBuf, KiSAbortContext const *tCtxt) {
    KI_ASSERT(mBuf != nullptr,  KiErr_InParameter);
    KI_ASSERT(tCtxt != nullptr, KiErr_InParameter);

    WCHAR *tmpBuf = KiPlatform_CreateFromKiraEncoding(mBuf, -1, KI_DONTCARE(KiTSize), KI_DONTCARE(KiTSize));
    {
        MessageBoxW(nullptr, tmpBuf, u"Debug Error", MB_OK);
    }
    KiPlatform_FreeString(tmpBuf);
}

KI_NORETURN KiTVoid KI_CALL KiPlatform_Exit(KiEErrorCode errCode) {
    ExitProcess((UINT)errCode);
}
/** \endcond */
#endif /* (defined KI_PLATFORM_WINDOWS) */
