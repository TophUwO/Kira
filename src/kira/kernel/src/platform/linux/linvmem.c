/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2026 TophUwO <tophuwo01@gmail.com>                                                                  *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

/**
 * \file  linvmem.c
 * \brief implements Linux-specific routines for virtual memory (VMEM) management
 */
#if (defined KI_PLATFORM_LINUX)


/* stdlib includes */
#include <stdint.h>

#include <errno.h>

/* Linux includes */
#include <unistd.h>

#include <sys/mman.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 * 
 */
static KiTBool KI_CALL KiInternal_IsPageAligned(KiTVoid const *addr) {
    KiTSize const pageSize = (KiTSize)sysconf(_SC_PAGESIZE);
    {
        KiTBool const isAligned = (uintptr_t)addr % pageSize == 0;

        return isAligned;
    }
}
/** \endcond */


KiEErrorCode KI_CALL KiPlatform_VirtualReserve(KiTVoid *stAddr, KiTSize reqSz, KiTVoid **resPtr) {
    KI_ASSERT(reqSz > 0,         KiErr_SizeParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    int const prot  = PROT_READ   | PROT_WRITE;
    int const flags = MAP_PRIVATE | MAP_ANONYMOUS;
    {
        *resPtr = (KiTVoid *)mmap((void *)stAddr, (size_t)reqSz, prot, flags, -1, 0);
    }

    return *resPtr != (KiTVoid *)MAP_FAILED
        ? KiErr_Ok
        : KiNativeErrorCodeToKiraErrorCode(errno)
    ;
}

KiEErrorCode KI_CALL KiPlatform_VirtualCommit(KiTVoid *stAddr, KiTSize reqSz, KiTVoid **resPtr) {
    KI_ASSERT(stAddr != nullptr,                           KiErr_InOutParameter);
    KI_ASSERT(KiInternal_IsPageAligned(stAddr) == KI_TRUE, KiErr_InOutParameter);
    KI_ASSERT(reqSz > 0,                                   KiErr_SizeParameter);
    KI_ASSERT(resPtr != nullptr,                           KiErr_OutptrParameter);

    /*
     * mmap() does not really support reserve + (explicit) commit like Windows' VirtualAlloc(). Instead, pages are
     * demand-paged. Thus, this function is superfluous. Simply do nothing for now.
     */
    *resPtr = stAddr;

    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiPlatform_VirtualUncommit(KiTVoid *stAddr, KiTSize reqSz) {
    KI_ASSERT(stAddr != nullptr,                           KiErr_InOutParameter);
    KI_ASSERT(KiInternal_IsPageAligned(stAddr) == KI_TRUE, KiErr_InOutParameter);
    KI_ASSERT(reqSz > 0,                                   KiErr_SizeParameter);

    /* mmap() also does not support uncommitting. Thus, do nothing. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiPlatform_VirtualFree(KiTVoid *stAddr, KiTSize reqSz) {
    KI_ASSERT(stAddr != nullptr,                           KiErr_InOutParameter);
    KI_ASSERT(KiInternal_IsPageAligned(stAddr) == KI_TRUE, KiErr_InOutParameter);
    KI_ASSERT(reqSz > 0,                                   KiErr_SizeParameter);

    int const res = munmap((void *)stAddr, (size_t)reqSz);

    return res == -1
        ? KiNativeErrorCodeToKiraErrorCode(errno)
        : (res != 0 ? KiErr_Unknown : KiErr_Ok)
    ;
}

KiTSize KI_CALL KiPlatform_VirtualGetPageSize(KiTVoid) {
    return (KiTSize)sysconf(_SC_PAGESIZE);
}


#endif /* KI_PLATFORM_LINUX */


