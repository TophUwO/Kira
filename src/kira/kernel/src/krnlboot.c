/*****************************************************************************************************************
 * Kira - cross-platform 2-D role-playing game (RPG) game engine for desktop and mobile, and console platforms *
 *                                                                                                               *
 * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                   *
 *                                                                                                               *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of  *
 * this project. If this file is not present, visit                                                              *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                               *
 *****************************************************************************************************************/

/** \cond INTERNAL */
/**
 * \file  krnlboot.c
 * \brief implements the kernel-level boot sequence
 */


/* Kira includes */
#include <kira/kernel/reg.h>

#include <kira/kernel/int/krnlboot.h>
#include <kira/kernel/int/krnlmod.h>


/** \cond INTERNAL */
KI_KRNLMOD_IMPORT(Registry);
KI_KRNLMOD_IMPORT(ErrorStringificationService);
KI_KRNLMOD_IMPORT(SystemDirectoryControl);
KI_KRNLMOD_IMPORT(PoolAllocator);
KI_KRNLMOD_IMPORT(ProfileManagement);
KI_KRNLMOD_IMPORT(ExceptionHandlingSystem);
KI_KRNLMOD_IMPORT(DebugModuleControl);


/**
 */
static KiSKrnlModuleInfo const *gl_c_SystemControlTable[] = {
    &KI_KRNLMOD_IDENTIFY(ErrorStringificationService),
    &KI_KRNLMOD_IDENTIFY(ExceptionHandlingSystem),
    &KI_KRNLMOD_IDENTIFY(DebugModuleControl),
    &KI_KRNLMOD_IDENTIFY(SystemDirectoryControl),
    &KI_KRNLMOD_IDENTIFY(Registry),
    &KI_KRNLMOD_IDENTIFY(PoolAllocator),
    &KI_KRNLMOD_IDENTIFY(ProfileManagement)
};

static KiTIndex gl_InitIndex = -1;
/** \endcond */


KiEErrorCode KI_CALL KiKrnlStartSystems(KiTVoid) {
    /* Go through all the kernel-level systems and start them up. */
    for (KiTIndex i = 0; i < (KiTIndex)KI_COUNTOF(gl_c_SystemControlTable); i++) {
        auto const *const currSysEntry = gl_c_SystemControlTable[i];

        /* Initialize the system. */
        KiEErrorCode errCode = (*currSysEntry->mp_fnInit)(nullptr);
        if (errCode != KiErr_Ok) {
            /*
             * If this fails, we uninitialize all systems that have been initialized already. We do not need to
             * uninitialize the system that failed because the initialization callback must guarantee to rollback
             * partial initialization in case of a failure.
             */
            KiKrnlShutdownSystems();

            return errCode;
        }

        /*
         * Increment the current initialization index. This is needed so that we know which systems we have already
         * initialized and can be safely uninitialized if the initialization routine fails.
         */
        ++gl_InitIndex;
    }

    /* Initialized all kernel-level systems. All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiKrnlShutdownSystems(KiTVoid) {
    /* Uninitalize all systems that have already been initialized. */
    for (KiTIndex i = gl_InitIndex; i >= 0; i--) {
        auto const *const currSysEntry = gl_c_SystemControlTable[i];

        /* Uninitialize the system. */
        (*currSysEntry->mp_fnUninit)(nullptr);
    }

    /* "Perfectly clean... as all things should be." */
    gl_InitIndex = -1;
    return KiErr_Ok;
}
/** \cond INTERNAL */


