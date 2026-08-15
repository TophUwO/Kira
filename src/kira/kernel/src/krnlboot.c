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
 * \file  krnlboot.c
 * \brief implements the kernel-level boot sequence
 */


/* Kira includes */
#include <kira/kernel/reg.h>

#include <kira/kernel/int/krnlmod.h>


/** \cond */
//KI_KRNLMOD_IMPORT(Registry);
KI_KRNLMOD_IMPORT(ErrorStringificationService);
KI_KRNLMOD_IMPORT(RuntimeConfiguration);
KI_KRNLMOD_IMPORT(ProfileManager);


/**
 */
static KiSModuleInfo const *const gl_c_ModuleInitTable[] = {
    &KI_KRNLMOD_IDENTIFY(ErrorStringificationService),
    &KI_KRNLMOD_IDENTIFY(RuntimeConfiguration),
    &KI_KRNLMOD_IDENTIFY(ProfileManager),
    //&KI_KRNLMOD_IDENTIFY(Registry)
};

/**
 */
static KiTIndex gl_InitIndex = -1;
/** \endcond */


KiEErrorCode KI_CALL KiStartKernelModules(KiTVoid) {
    for (KiTIndex i = 0; i < (KiTIndex)KI_COUNTOF(gl_c_ModuleInitTable); i++) {
        KiSModuleInfo const *const currModEntry = gl_c_ModuleInitTable[i];

        /* Initialize the module. */
        KiEErrorCode errCode = (*currModEntry->mp_fnInit)(nullptr);
        if (errCode != KiErr_Ok) {
            /** \cond */
            KI_NATIVE extern KiEErrorCode KI_CALL KiShutdownKernelModules(KiTVoid);
            /** \endcond */

            /*
             * If this fails, we uninitialize all modules that have been initialized already. We do not need to
             * uninitialize the system that failed because the initialization callback must guarantee to rollback
             * partial initialization in case of a failure.
             */
            KiShutdownKernelModules();

            return errCode;
        }

        ++gl_InitIndex;
    }

    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiShutdownKernelModules(KiTVoid) {
    /* Uninitalize all modules that have already been initialized. */
    for (KiTIndex i = gl_InitIndex; i >= 0; i--) {
        KiSModuleInfo const *const currSysEntry = gl_c_ModuleInitTable[i];

        (*currSysEntry->mp_fnUninit)(nullptr);
    }

    /* "Perfectly clean... as all things should be." */
    gl_InitIndex = -1;
    return KiErr_Ok;
}
/** \endcond */
