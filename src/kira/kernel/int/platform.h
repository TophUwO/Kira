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
 * \file  platform.h
 * \brief defines the API for the kernel-level platform layer
 *
 * The routines implemented here must be implemented per platform and according to the requirements denoted in their
 * respective documentation.
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/dbg.h>

#include <kira/kernel/int/sync.h>

/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_AllocateString(KiTSize sizeInBytes) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_FreeString(KiTVoid *strPtr) KI_VIRTUAL;

/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiPlatform_CreateFromKiraEncoding(KiTChar const *srcPtr, KiTSize *sizePtr) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_CreateFromNativeEncoding(KiTVoid const *srcPtr, KiTSize *sizePtr) KI_VIRTUAL;

/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_GetCurrentWorkingDirectory(KiTVoid) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_SetCurrentWorkingDirectory(KiTChar const *newWdPath) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_GetApplicationRootDirectory(KiTSize *sizePtr, KiTSize *lenPtr) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_CanonicalizeSeparators(KiTChar *bufPtr) KI_VIRTUAL;

/**
 */
KI_NATIVE extern KiTDynLibHandle KI_CALL KiPlatform_LoadLibrary(KiTChar const *libPath) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_UnloadLibrary(KiTDynLibHandle libHandle) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiSFunctionHandle KI_CALL KiPlatform_ResolveSymbol(KiTDynLibHandle libHandle, KiTChar const *symName) KI_VIRTUAL;

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_VirtualReserve(KiTVoid *stAddr, KiTSize reqSz, KiTVoid **resPtr) KI_VIRTUAL;
/**
 * must return zeroed memory on all platforms
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_VirtualCommit(KiTVoid *stAddr, KiTSize reqSz, KiTVoid **resPtr) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_VirtualUncommit(KiTVoid *stAddr, KiTSize reqSz) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_VirtualFree(KiTVoid *stAddr, KiTSize reqSz) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiPlatform_VirtualGetPageSize(KiTVoid) KI_VIRTUAL;

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiVirtual_RWLockCreate(KiTRWLockHandle *resPtr) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockDestroy(KiTRWLockHandle *rwLockPtr) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockAcquireRead(KiTRWLockHandle *rwLockPtr) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockReleaseRead(KiTRWLockHandle *rwLockPtr) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockAcquireWrite(KiTRWLockHandle *rwLockPtr) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockReleaseWrite(KiTRWLockHandle *rwLockPtr) KI_VIRTUAL;

/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_Notify(KiTChar const *mBuf, KiSDebugTerminationContext const *tCtxt) KI_VIRTUAL;
/**
 */
KI_NATIVE extern KI_NORETURN KiTVoid KI_CALL KiPlatform_Exit(KiEErrorCode errCode) KI_VIRTUAL;

/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_Sleep(KiTUint32 sleepMs) KI_VIRTUAL;


