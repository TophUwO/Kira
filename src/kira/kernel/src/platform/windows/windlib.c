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
 * \file  windlib.c
 * \brief implements the platform-dependent functions (this file is Windows(R)-specific) for the dynamic library service
 */
#if (defined KI_PLATFORM_WINDOWS)


/* stdlib includes */
#include <stdlib.h>
#include <string.h>

/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/kernel/int/platform.h>


KiTDynLibHandle KI_CALL KiPlatform_LoadLibrary(KiTChar const *libPath) {
    /* Convert to UTF-16 because that's what Windows likes. */
    WCHAR *u16Path = KiPlatform_CreateFromKiraEncoding(libPath, -1, KI_DONTCARE(KiTSize), KI_DONTCARE(KiTSize));
    if (u16Path == nullptr)
        return nullptr;

    /* Actually load library. */    
    KiTDynLibHandle res = (KiTDynLibHandle)LoadLibraryW((LPCWSTR)u16Path);

    /* Clean up. */
    free(u16Path);
    return res;
}

KiTVoid KI_CALL KiPlatform_UnloadLibrary(KiTDynLibHandle libHandle) {
    FreeLibrary((HMODULE)libHandle);
}

KiSFunctionHandle KI_CALL KiPlatform_ResolveSymbol(KiTDynLibHandle libHandle, KiTChar const *symName) {
    if (libHandle == nullptr)
        return (KiSFunctionHandle){ nullptr };
    
    return (KiSFunctionHandle const){ 
        .mp_fnPtr = (KiTVoid (KI_CALL *)(KiTVoid))GetProcAddress((HMODULE)libHandle, (LPCSTR)symName)
    };
}

KiTBool KI_CALL KiPlatform_IsLibrary(KiTChar const *filePath) {
    KiTChar extBuf[64] = { 0 };
    _splitpath_s(filePath, NULL, 0, NULL, 0, NULL, 0, extBuf, sizeof extBuf);

    return !strcmp(extBuf, ".dll") || !strcmp(extBuf, ".DLL");
}


#endif /* (defined KI_PLATFORM_WINDOWS) */
