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
 * \file  krnlmod.h
 * \brief defines the annotations and the API for the modular kernel
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 */
#define KI_KRNLMOD_IDENTIFY(modId) gl_c_##modId##_Info
/**
 */
#define KI_KRNLMOD_DEFINE(modId)   KiSModuleInfo const KI_KRNLMOD_IDENTIFY(modId) =
/**
 */
#define KI_KRNLMOD_IMPORT(modId)   extern KiSModuleInfo const KI_KRNLMOD_IDENTIFY(modId)

/**
 */
#define KI_KRNLMOD_INITFN(modId)   KiInternal_Init##modId
/**
 */
#define KI_KRNLMOD_UNINITFN(modId) KiInternal_Uninit##modId


/**
 */
KI_NATIVE typedef struct KiSModuleInfo {
    KiTSize              m_structSize;
    KiSUuid       const *mp_modUuid;
    KiSStringView const *mp_modId;
    KiTFlags64           m_modFlags;

    KiEErrorCode (KI_CALL *mp_fnInit)(KiTVoid *extraParam);
    KiEErrorCode (KI_CALL *mp_fnUninit)(KiTVoid *extraParam);
} KiSModuleInfo;


