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
 * \file  winvmem.c
 * \brief implements the native virtual memory management functions for the Windows(R) platform
 */
#if (defined KI_PLATFORM_WINDOWS)


/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/kernel/dbg.h>


KiEErrorCode KI_CALL KiPlatform_VirtualReserve(KiTVoid *stAddr, KiTSize reqSz, KiTVoid **resPtr) {
    KI_ASSERT(reqSz > 0,         KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    return (*resPtr = VirtualAlloc(stAddr, reqSz, MEM_RESERVE, PAGE_READWRITE)) != nullptr
        ? KiErr_Ok
        : KiErr_VirtualReserve
    ;
}

KiEErrorCode KI_CALL KiPlatform_VirtualCommit(KiTVoid *stAddr, KiTSize reqSz, KiTVoid **resPtr) {
    KI_ASSERT(stAddr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(reqSz > 0,         KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    return (*resPtr = VirtualAlloc(stAddr, reqSz, MEM_COMMIT, PAGE_READWRITE)) != nullptr
        ? KiErr_Ok
        : KiErr_VirtualCommit
    ;
}

KiEErrorCode KI_CALL KiPlatform_VirtualUncommit(KiTVoid *stAddr, KiTSize reqSz) {
    KI_ASSERT(stAddr != nullptr, KiErr_InOutParameter);

    if (reqSz == 0)
        return KiErr_NoOperation;

    return VirtualFree(stAddr, reqSz, MEM_DECOMMIT)
        ? KiErr_Ok
        : KiErr_VirtualUncommit
    ;
}

KiEErrorCode KI_CALL KiPlatform_VirtualFree(KiTVoid *stAddr, KiTSize reqSz) {
    KI_UNREFERENCED_PARAMETER(reqSz);
    
    return VirtualFree(stAddr, 0, MEM_RELEASE)
        ? KiErr_Ok
        : KiErr_VirtualFree
    ;
}

KiTSize KI_CALL KiPlatform_VirtualGetPageSize(KiTVoid) {
    /* Get page size from system. This also works if large pages are enabled. */
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    return (KiTSize)si.dwPageSize;
}


#endif /* (defined KI_PLATFORM_WINDOWS) */
