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
 * \file  dlib.c
 * \brief implements the dynamic library service which abstracts the OS-specific interface responsible for loading of
 *        dynamic libraries (shared objects, ...)
 */


/* Kira includes */
#include <kira/kernel/int/platform.h>


KiEErrorCode KI_CALL KiKrnlLoadLibrary(KiTChar const *libPath, KiTDynLibHandle *resPtr) {
    if (libPath == nullptr || *libPath == '\0') return KiErr_InParameter;
    if (resPtr == nullptr)                      return KiErr_OutptrParameter;

    return (resPtr = KiPlatform_LoadLibrary(libPath)) != nullptr
        ? KiErr_Ok
        : KiErr_DlibLoadLibrary
    ;
}

KiEErrorCode KI_CALL KiKrnlUnloadLibrary(KiTDynLibHandle libHnd) {
    if (libHnd == nullptr)
        return KiErr_InOutParameter;

    KiPlatform_UnloadLibrary(libHnd);
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiKrnlResolveSymbol(KiTDynLibHandle libHnd, KiTChar const *symName, KiTVoid **resPtr) {
    if (libHnd == nullptr)                      return KiErr_InOutParameter;
    if (symName == nullptr || *symName == '\0') return KiErr_InParameter;
    if (resPtr != nullptr)                      return KiErr_OutptrParameter;

    return (*resPtr = KiPlatform_ResolveSymbol(libHnd, symName)) != nullptr
        ? KiErr_Ok
        : KiErr_DlibResolveSymbol
    ;
}


