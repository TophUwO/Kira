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
 * \file  krnlmod.h
 * \brief defines the annotations and the API for the modular kernel
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 */
#define KI_KRNLMOD(modId, deps, ...)                                                                           \
    static KiSStaticArray const *gl_c_##modId##_Dependencies = &KI_MAKE_STATIC_ARRAY((KiSUuid const *[])deps); \
    static KiSStaticArray const *KI_CALL KiInternal_GetDeps##modId(KiTVoid) {                                  \
        return gl_c_##modId##_Dependencies;                                                                    \
    }                                                                                                          \
    __attribute__((section("KIRA_KMD"), used)) static KiSKernelModuleInfo const gl_c_##modId##_Info = {        \
        .ma_startMagic = {  '_', '_', 'A', 'n', 'a', 'r', 'c', 'h', 'i', 'a', 'M', 'a', 'm', 'a', '_', '_' },  \
        .mp_fnGetDeps  = &KiInternal_GetDeps##modId,                                                           \
        .m_moduleInfo  = __VA_ARGS__                                                                           \
    }

/**
 * 
 */
#define KI_KRNLMOD_DEPENDENCY(a, b, c, d) &KI_MAKE_UUID(a, b, c, d)
/**
 */
#define KI_KRNLMOD_INITFN(modId)          KiInternal_Init##modId
/**
 */
#define KI_KRNLMOD_UNINITFN(modId)        KiInternal_Uninit##modId


/**
 */
KI_NATIVE typedef struct KiSKernelModuleInfo {
    KiTByte const ma_startMagic[16];

    KiSStaticArray const *(KI_CALL *mp_fnGetDeps)(KiTVoid);
    struct {
        KiSUuid        const  *mp_modUuid;
        KiSStringView  const  *mp_modId;
        KiTFlags64             m_modFlags;
        KiSStaticArray const **mpp_depArr;

        KiEErrorCode (KI_CALL *mp_fnInit)(KiTVoid *extraParam);
        KiEErrorCode (KI_CALL *mp_fnUninit)(KiTVoid *extraParam);
    } m_moduleInfo;
} KiSKernelModuleInfo;


