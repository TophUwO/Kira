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
 * \file  winenv.c
 * \brief implements the retrieval routines for command-line and environment variables for the Windows platform
 */
#if (defined KI_PLATFORM_WINDOWS)


/* Windows includes */
#include <windows.h>

/* stdlib includes */
#include <stdlib.h>

#include <wchar.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/int/platform.h>


KiTChar *KI_CALL KiPlatform_GetCommandLine(KiTSize *sizePtr) {
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);

    return KiPlatform_CreateFromNativeEncoding((KiTVoid const *)GetCommandLineW(), -1, sizePtr, KI_DONTCARE(KiTSize));
}

KiTChar *KI_CALL KiPlatform_GetEnvironmentVariables(KiTSize *sizePtr) {
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);

    KiTChar *res;
    WCHAR *rawEnv = GetEnvironmentStringsW();
    {
        if (rawEnv == nullptr) {
            *sizePtr = 0;

            return nullptr;
        }

        /*
         * Since the return value is a block to a memory region where each environment variable is separated by one
         * NUL-terminator (with two consecutive NUL-terminators marking the end of the block), we need to determine the
         * size of the environment block manually.
         */
        KiTSize blockSize;
        for (WCHAR *currTerm = rawEnv, *nextTerm = nullptr;;) {
            /** \cond */
            /**
             * \brief maximum size of one <tt>K=V</tt> pair (<tt>0 > |V| >= 32768</tt>, see documentation linked below
             *        for more information); simply double the size to (heuristically) account for the <tt>K=</tt> part
             * \sa    https://learn.microsoft.com/en-us/windows/win32/api/processenv/nf-processenv-getenvironmentvariablew
             */
            static KiTSize const gl_c_MaxEnvAdvance = 2 * 32768;
            /** \endcond */

            if ((nextTerm = wmemchr(currTerm, u'\0', gl_c_MaxEnvAdvance)) == nullptr) {
                /* K=V pair is too long. Simply use the block up to the last correctly parsed one. */
                blockSize = (KiTSize)((KiTIntptr)currTerm - (KiTIntptr)rawEnv);

                break;
            } else if (nextTerm[1] == u'\0') {
                /*
                 * We found two consecutive NUL-terminators in a row. We're done. The peek by one character is safe
                 * because the environment block always ends in \0\0 and wmemchr() points to the first \0.
                 */
                blockSize = (KiTSize)(((KiTIntptr)nextTerm + 2) - (KiTIntptr)rawEnv);

                break;
            }

            /* Only found a single \0. Continue search. */
            currTerm = nextTerm + 1;
        }

        /*
         * Finally, convert the entire block at once. This function requires the size in platform characters, not
         * necessarily in bytes. Thus, we need to reduce the size down to platform characters.
         */
        res = KiPlatform_CreateFromNativeEncoding(rawEnv, blockSize >> KI_P2LOG2(sizeof(WCHAR)), sizePtr, KI_DONTCARE(KiTSize));
    }
    FreeEnvironmentStringsW(rawEnv);

    return res;
}

/**
 * \ki_tested_on{Windows; tested;
 *  <b>The following test cases have been successfully verified:</b>
 *  <ul>
 *   <li>assertion failure upon passing an invalid parameter</li>
 *   <li>non-interfering operation (no writes to \c name by another thread while this function is being executed on the current thread)</li>
 *   <li>interfering operation (value of environment variable changed by another thread while executing this function)</li>
 *  </ul>;
 *  03/06/2026
 * }
 */
KiTChar *KI_CALL KiPlatform_GetEnvironmentVariable(KiTChar const *name, KiTSize *sizePtr) {
    KI_ASSERT(name != nullptr,    KiErr_InParameter);
    KI_ASSERT(*name != '\0',      KiErr_InParameter);
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);

    /* (1) Convert name to native encoding. */
    KiTVoid *ntVarId = KiPlatform_CreateFromKiraEncoding(name, -1, KI_DONTCARE(KiTSize), KI_DONTCARE(KiTSize));
    if (ntVarId == nullptr)
        return nullptr;

    /*
     * (2) Get the required buffer size to hold the environment variable's contents in native encoding. Do this in a
     *     loop because another thread might change (or possibly delete) the environment variable between both calls to
     *     GetEnvironmentVariableW().
     *     This loop pattern ensures that we will always get a buffer large enough to hold the value of the variable as
     *     of the second call to GetEnvironmentVariableW(). Retry at most 50 times.
     */
    KiTSize nTries = 50;
    do {
        SetLastError(0);
        DWORD reqSize = GetEnvironmentVariableW((LPCWSTR)ntVarId, nullptr, 0);
        if (reqSize == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
            KiPlatform_FreeString(ntVarId);
     
            return nullptr;
        }

        /* (3) Get the value and convert to Kira encoding. */
        WCHAR *rawVal = malloc(sizeof *rawVal * reqSize);
        {
            if (rawVal == nullptr) {
                KiPlatform_FreeString(ntVarId);

                return nullptr;
            }

            /*
             * We check error again because, in theory, between the two calls to GetEnvironmentVariableW(), another thread
             * might have changed its value or even deleted it.
             */
            DWORD const newReqSize = GetEnvironmentVariableW((LPCWSTR)ntVarId, rawVal, reqSize);
            if (newReqSize == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
                /* Return instantly if the variable was deleted. */
                free(rawVal);

                break;
            } else if (newReqSize < reqSize) {
                KiTChar *resPtr = KiPlatform_CreateFromNativeEncoding(rawVal, -1, sizePtr, KI_DONTCARE(KiTSize));

                free(rawVal);
                KiPlatform_FreeString(ntVarId);
                return resPtr;
            }
        }
        /* If we arrive here, the value has been changed. Retry. */
        free(rawVal);
    } while (nTries-- > 0);

    /* Error (alloc, encoding, not found, etc.) or max no. of tries reached. */
    KiPlatform_FreeString(ntVarId);
    return nullptr;
}

/**
 * \ki_tested_on{Windows; tested;
 *  <b>The following test cases have been successfully verified:</b>
 *  <ul>
 *   <li>assertion failure upon entering invalid parameter as per preconditions</li>
 *   <li>adding environment variable</li>
 *   <li>updating environment variable</li>
 *   <li>unsetting (i.e., removing) environment variable</li>
 *  </ul><br>
 *  <b>Not yet successfully verified:</b>
 *  <ul>
 *   <li>encoding error in \c name or <tt>value</tt></li>
 *  </ul>;
 *  03/07/2026
 * }
 */
KiTBool KI_CALL KiPlatform_SetEnvironmentVariable(KiTChar const *name, KiTChar const *value) {
    KI_ASSERT(name != nullptr, KiErr_InParameter);

    /*
     * (1) Get the size of the buffer required to hold the K-V pair. The pair must be in the form of "K\0V\0" so that we
     *     can access both strings as if they were NUL-terminated. We also handle the special case of *value* being
     *      *nullptr*. this case, the behavior is to unset the variable. This must be done by calling SetEnvironmentVariableW() 
     *     directly with *name* and *nullptr* for *value*. In such a case, we simply use the buffer to store the
     *     converted name only.
     */
    KiTSize keySize = 0, valSize = 0;
    {
        /*
         * When *cbMultiByte* is -1, the string is expected to be NUL-terminated. As a result, the return value includes
         * this NUL-terminator.
         */
        keySize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, name, -1, nullptr, 0);
        if (value != nullptr)
            valSize = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value, -1, nullptr, 0);

        if (keySize == 0 || (value != nullptr && valSize == 0))
            return KI_FALSE;
    }

    /* (2) Allocate the buffer. */
    WCHAR *kvPairBlock = malloc((keySize + valSize) * sizeof *kvPairBlock);
    if (kvPairBlock == nullptr)
        return KI_FALSE;

    /* (3) Format the K\0V\0 pair. */
    MultiByteToWideChar(CP_UTF8, 0, name, -1, &kvPairBlock[0], keySize);
    if (value != nullptr)
        MultiByteToWideChar(CP_UTF8, 0, value, -1, &kvPairBlock[keySize], valSize);

    /* (4) Now, set or unset the variable. */
    BOOL const res = SetEnvironmentVariableW(&kvPairBlock[0], value != nullptr ? &kvPairBlock[keySize] : nullptr);

    free(kvPairBlock);
    return (KiTBool)res;
}


#endif /* defined KI_PLATFORM_WINDOWS */


