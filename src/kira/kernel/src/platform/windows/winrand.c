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
 * \file  winrand.c
 * \brief implements native random number seed generation for the Kicrosoft(R) Windows(R) platform
 */

#if (defined KI_PLATFORM_WINDOWS)
/* Windows includes */
#include <windows.h>
#include <wincrypt.h>

/* stdlib includes */
#include <stdint.h>

/* Kira includes */
#include <kira/kcm.h>
#include <kira/dbg.h>


/* static library bindings */
#pragma comment (lib, "crypt32.lib")


KiEErrorCode KI_CALL KiPlatform_GetRandomBytes(KiTSize sizeInBytes, KiTVoid *resPtr) {
    KI_ASSERT(sizeInBytes > 0,   KiErr_SizeParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutParameter);

    /* (1) Acquire cryptographic provider context. */
    HCRYPTPROV cryptProv;
    if (CryptAcquireContextW(&cryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE) {
        memset((void *)resPtr, 0, (size_t)sizeInBytes);

        return KiErr_CannotGenRandom;
    }

    /* (2) Fill the provided memory buffer with high-quality random bytes. */
    BOOL const retVal = CryptGenRandom(cryptProv, (DWORD)sizeInBytes, (BYTE *)resPtr);
    
    /* (3) Release the context. */
    CryptReleaseContext(cryptProv, 0);
    if (retVal != FALSE) {
        memset((void *)resPtr, 0, (size_t)sizeInBytes);

        return KiErr_CannotGenRandom;
    }

    return KiErr_Ok;
}
#endif /* (defined KI_PLATFORM_WINDOWS) */
