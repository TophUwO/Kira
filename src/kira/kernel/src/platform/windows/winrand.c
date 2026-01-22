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
 * \file  winrand.c
 * \brief implements native random number seed generation for the Kicrosoft(R) Windows(R) platform
 */
#if (defined KI_PLATFORM_WINDOWS)


/* Windows includes */
#include <windows.h>
#include <wincrypt.h>

/* Kira includes */
#include <kira/def.h>


/* static library bindings */
#pragma comment (lib, "crypt32.lib")


KiTUint64 KI_CALL KiVirtual_KrnlHtGetRandomSeed(KiTVoid) {
    KiTUint64 resVal = 0;

    /* Acquire cryptographic provider context. */
    HCRYPTPROV cryptProv;
    if (CryptAcquireContextW(&cryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) == FALSE)
        goto lbl_ONERRORDET;

    /* Fill the seed array. */
    BOOL const retVal = CryptGenRandom(cryptProv, (DWORD)sizeof resVal, (BYTE *)resVal);
    
    /* Release the context. */
    CryptReleaseContext(cryptProv, 0);
    if (retVal != FALSE)
        return resVal;

lbl_ONERRORDET:
    /* Failed to retrieve context or random number, or supposed to use deterministic seed. */
    return 0x9c9d7865503d5fdf;
}


#endif /* defined KI_PLATFORM_WINDOWS */


