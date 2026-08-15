/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

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
#include <kira/dbg.h>

#include <kira/kernel/int/sync.h>


/**
 * \defgroup KiPlatfLayer Kira Platform Layer
 *
 * Kira is a cross-platform software development framework. It is designed to be able to run in various environments.
 * Not all of these environments are the same: Different platforms expose differing ways of interacting with them. For
 * this reason, Kira has, aside from the \e Kernel- and <em>User Layer</em>, a third layer, the <em>Platform Layer</em>.
 * This layer is essentially a set of APIs that create a compatibility layer between Kira and the native platform. They
 * handle things like string conversion and parameter translation as well as the translation of return values to a
 * "Kira-compatible" format.<br>
 * In order to port Kira to a new platform, in theory (and hopefully also practice), all the programmer has to do is
 * reimplement the functions that are contained inside this group according to their specifications. For this reason,
 * the documentation for the APIs in the Platform Layer are extra detailed.
 */
/** \{ */
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
 * \brief   retrieves the value of the environment variable identified by \c name of the current process
 * \param   [in] name pointer to a (<tt>NUL</tt>-terminated) C-string containing the name of the environment variable
 *               of which the value is to be retrieved
 * \param   [out] sizePtr pointer to a variable of type \c KiTSize which will receive the number of bytes (incl. <tt>NUL</tt>)
 *                in the buffer that represent the result
 * \returns pointer to a C-string containing the value of the environment variable \c name, or \c nullptr if there was
 *          an error or \c name could not be found in the current environment
 * \pre     Parameters \c name and \c sizePtr must not be \c nullptr.
 * \pre     Parameter \c name must not be the empty string.
 * \post    If the function succeeds, \c sizePtr is set to the size in bytes of the returned memory block. If the
 *          function fails, the value of \c *sizePtr is undefined.
 * \note    If the function succeeds, it dynamically allocates memory. When you are done working with the string
 *          returned by this function, call \c KiPlatform_FreeString() in order to destroy it.
 * \note    This function queries the \b current value of the given environment variable. If its value gets changed
 *          throughout the application's run, the value returned by this function will likely become outdated.
 * \note    This function must be thread-safe under the assumption that neither \c name nor \c sizePtr are written to
 *          by another thread at the same time this function is being executed on the current thread.
 * \warning The actual query for the value of the environment variable is not guaranteed to be thread-safe.
 * \warning If either \c name or \c sizePtr is modified by another thread while this function is being executed, the
 *          behavior is undefined.
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_GetEnvironmentVariable(KiTChar const *name, KiTSize *sizePtr) KI_PLATFORM;
/**
 * \brief   updates the value of the environment variable identified by \c name of the current process with \c value
 * \param   [in] name pointer to a (<tt>NUL</tt>-terminated) C-string containing the name of the environment variable
 *               of which the value is to be updated
 * \param   [in] value pointer to a (<tt>NUL</tt>-terminated) C-string containing the value to update \c name with;
 *               this parameter can be \c nullptr in which case the variable with name \c name will be \e unset (i.e.,
 *               <tt>removed</tt>)
 * \returns \c KI_TRUE on success, or \c KI_FALSE on failure
 * \pre     Parameter \c name must be non-<tt>nullptr</tt> and non-empty.
 * \post    If the function succeeds (i.e., returns <tt>KI_TRUE</tt>), the value of \c name is updated with \c value.
 *          If \c value is \c nullptr, the variable identified by \c name is removed from the current process
 *          environment.
 *          If no environment variable with name \c name existed prior to calling this function, a new variable with
 *          name \c name is added and set to \c value (unless \c value is <tt>nullptr</tt>.) If no variable with name
 *          \c name existed prior to calling this function and \c value is \c nullptr, the environment is not changed
 *          and the function returns \c KI_TRUE.
 * \note    This function must be thread-safe under the assumption that neither \c name nor \c value are written to by
 *          another thread at the same time this function is being executed on the current thread.
 * \warning If either \c name or \c value are changed by another thread while this function is being executed, the
 *          behavior is undefined.
 */
KI_NATIVE extern KiTBool KI_CALL KiPlatform_SetEnvironmentVariable(KiTChar const *name, KiTChar const *value) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTChar KI_CALL KiPlatform_GetPathSeparator(KiTVoid) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTChar *KI_CALL KiPlatform_CanonicalizeSeparators(KiTChar *pathStr) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTBool KI_CALL KiPlatform_PathExists(KiTChar const *pathStr, KiTBool isDir) KI_PLATFORM;
/**
 */
KI_NATIVE extern KiTBool KI_CALL KiPlatform_IsPathRelative(KiTChar const *pathStr) KI_PLATFORM;

/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_OpenFile(KiTChar const *pathStr, KiTChar const *modeStr, KiTVoid **resPtr) KI_PLATFORM;
/**
 * 
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_CloseFile(KiTVoid *fHandle) KI_PLATFORM;
/**
 * 
 */
KI_NATIVE extern KiTSize KI_CALL KiPlatform_GetFileSize(KiTVoid *fHandle) KI_PLATFORM;
/**
 * 
 */
KI_NATIVE extern KiTOffset KI_CALL KiPlatform_GetFilePosition(KiTVoid *fHandle) KI_PLATFORM;
/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_SetFilePosition(KiTVoid *fHandle, KiTOffset offset, KiEFilePositionOrigin origin) KI_PLATFORM;
/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_ReadFromFile(KiTVoid *fHandle, KiTVoid *dstBufPtr, KiTSize sizeInBytes, KiTSize *resSize) KI_PLATFORM;
/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_WriteToFile(KiTVoid *fHandle, KiTVoid const *srcBufPtr, KiTSize sizeInBytes) KI_PLATFORM;

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
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_Notify(KiTChar const *mBuf, KiSAbortContext const *tCtxt) KI_PLATFORM;
/**
 */
KI_NATIVE extern KI_NORETURN KiTVoid KI_CALL KiPlatform_Exit(KiEErrorCode errCode) KI_PLATFORM;

/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_Sleep(KiTUint32 sleepMs) KI_PLATFORM;

/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_GetRandomBytes(KiTSize sizeInBytes, KiTVoid *resPtr) KI_PLATFORM;
/** \} */
/** \endcond */
