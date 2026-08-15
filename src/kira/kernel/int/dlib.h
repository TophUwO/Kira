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
 * \file  dlib.h
 * \brief defines the internal kernel-level API dedicated to loading module files
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiLoadLibrary(KiTChar const *libPath, KiTDynLibHandle *resPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiUnloadLibrary(KiTDynLibHandle libHnd);

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiResolveSymbol(
    KiTDynLibHandle libHnd,
    KiTChar const *symName,
    KiTVoid **resPtr
);


