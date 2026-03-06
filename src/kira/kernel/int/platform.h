/*****************************************************************************************************************
 * Kira - cross-platform 2-D role-playing game (RPG) game engine for desktop and mobile, and console platforms *
 *                                                                                                               *
 * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                   *
 *                                                                                                               *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of  *
 * this project. If this file is not present, visit                                                              *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                               *
 *****************************************************************************************************************/

/** \cond INTERNAL */
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
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_AllocateString(KiTSize sizeInBytes) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_FreeString(KiTVoid *strPtr) KI_PLATFORM;

/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiPlatform_CreateFromKiraEncoding(
    KiTChar const *srcPtr,
    KiTInt64 maxLen,
    KiTSize *sizePtr,
    KiTSize *memSizePtr
) KI_PLATFORM;
/**
 */
KiTChar *KI_CALL KiPlatform_CreateFromNativeEncoding(
    KiTVoid const *srcPtr,
    KiTInt64 maxLen,
    KiTSize *sizePtr,
    KiTSize *memSizePtr
) KI_PLATFORM;

/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_GetCurrentWorkingDirectory(KiTVoid) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_SetCurrentWorkingDirectory(KiTChar const *newWdPath) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_GetApplicationRootDirectory(KiTSize *sizePtr, KiTSize *lenPtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_GetCommandLine(KiTSize *sizePtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_GetEnvironmentVariables(KiTSize *sizePtr) KI_PLATFORM;
/**
 * \brief   retrieves the value of the environment variable identified by \c varIdent
 * \param   [in] varIdent pointer to a (<tt>NUL</tt>-terminated) C-string containing the name of the environment variable
 *               of which the value is to be retrieved
 * \param   [out] sizePtr pointer to a variable of type \c KiTSize which will receive the number of bytes (incl. <tt>NUL</tt>)
 *                in the buffer that represent the result
 * \returns pointer to a C-string containing the value of the environment variable \c varIdent, or \c nullptr if there
 *          was an error or \c varIdent could not be found in the current environment
 * \note    In case of an error, the function returns \c nullptr and the value of \c sizePtr is undefined.
 * \note    If the function succeeds, it dynamically allocates memory. When you are done working with the string returned
 *          by this function, call \c KiPlatform_FreeString() in order to destroy it.
 * \warning This function queries the \b current value of the given environment variable. If its value gets changed
 *          throughout the application's run, the value returned by this function will likely become outdated.
 *
 * \ki_tested{tested;
 *  <b>The following test cases have been successfully verified:</b>
 *  <ul>
 *   <li>assertion failure upon passing an invalid parameter</li>
 *   <li>non-interfering operation (no writes to \c varIdent by another thread while this function is being executed on the current thread)</li>
 *   <li>interfering operation (value of environment variable changed by another thread while executing this function)</li>
 *  </ul>;
 * 03/06/2026}
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_GetEnvironmentVariable(KiTChar const *varIdent, KiTSize *sizePtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_CanonicalizeSeparators(KiTChar *bufPtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTBool KI_CALL KiPlatform_PathExists(KiTChar const *pathStr, KiTBool isDir) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTBool KI_CALL KiPlatform_IsPathRelative(KiTChar const *pathStr) KI_PLATFORM;

/**
 */
KI_NATIVE extern KiTDynLibHandle KI_CALL KiPlatform_LoadLibrary(KiTChar const *libPath) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_UnloadLibrary(KiTDynLibHandle libHandle) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiSFunctionHandle KI_CALL KiPlatform_ResolveSymbol(KiTDynLibHandle libHandle, KiTChar const *symName) KI_PLATFORM;

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_VirtualReserve(KiTVoid *stAddr, KiTSize reqSz, KiTVoid **resPtr) KI_PLATFORM;
/**
 * must return zeroed memory on all platforms
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_VirtualCommit(KiTVoid *stAddr, KiTSize reqSz, KiTVoid **resPtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_VirtualUncommit(KiTVoid *stAddr, KiTSize reqSz) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_VirtualFree(KiTVoid *stAddr, KiTSize reqSz) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiPlatform_VirtualGetPageSize(KiTVoid) KI_PLATFORM;

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiVirtual_RWLockCreate(KiTRWLockHandle *resPtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockDestroy(KiTRWLockHandle *rwLockPtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockAcquireRead(KiTRWLockHandle *rwLockPtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockReleaseRead(KiTRWLockHandle *rwLockPtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockAcquireWrite(KiTRWLockHandle *rwLockPtr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiVirtual_RWLockReleaseWrite(KiTRWLockHandle *rwLockPtr) KI_PLATFORM;

/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_Notify(KiTChar const *mBuf, KiSDebugTerminationContext const *tCtxt) KI_PLATFORM;
/**
 */
KI_NATIVE extern KI_NORETURN KiTVoid KI_CALL KiPlatform_Exit(KiEErrorCode errCode) KI_PLATFORM;

/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_Sleep(KiTUint32 sleepMs) KI_PLATFORM;
/** \endcond */


