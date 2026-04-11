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
 * \file  linenv.c
 * \brief implements the Linux-specific routines for the management of environment variables
 */
#if (defined KI_PLATFORM_LINUX)


/* stdlib includes */
#include <stdlib.h>

#include <string.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


KiTChar *KI_CALL KiPlatform_GetEnvironmentVariable(KiTChar const *name, KiTSize *sizePtr) {
    KI_ASSERT(name != nullptr,    KiErr_InParameter);
    KI_ASSERT(*name != '\0',      KiErr_InParameter);
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);

    /* (1) Get the value of the environment variable. */
    char const *res = getenv(name);
    if (res == nullptr) {
        *sizePtr = 0;

        return nullptr;
    }
    KiTSize const len = strlen(res);

    /* (2) Copy it into a Kira string. This string must then be deallocated using KiPlatform_FreeString(). */
    KiTChar *cpyStr = KiPlatform_AllocateString(len + 1);
    if (cpyStr == nullptr) {
        *sizePtr = 0;

        return nullptr;
    }
    strcpy((char *)cpyStr, res);

    *sizePtr = len + 1;
    return cpyStr;
}

KiTBool KI_CALL KiPlatform_SetEnvironmentVariable(KiTChar const *name, KiTChar const *value) {
    KI_ASSERT(name != nullptr, KiErr_InParameter);
    KI_ASSERT(*name != '\0',   KiErr_InParameter);

    /* If *value* is nullptr, we unset the variable. If the variable does not exist, it is not an error. */
    if (value == nullptr) {
        int const res = unsetenv(name);
        
        return (KiTBool)(res != -1);
    }

    /* Update the value of *name*. If *name* does not exist, it is added to the environment. */
    return setenv(name, value, KI_TRUE) != -1;
}


#endif /* KI_PLATFORM_LINUX */


