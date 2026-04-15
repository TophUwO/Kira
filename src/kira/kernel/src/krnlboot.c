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


/* stdlib includes */
#include <string.h>
#include <stdio.h>

/* Kira includes */
#include <kira/kernel/int/krnlmod.h>


/** \cond INTERNAL */
#if (defined KI_HOST_GCC)
/** \cond */
/**
 * 
 */
extern KiTByte const __start_KIRA_KMD[];
/**
 * 
 */
extern KiTByte const __stop_KIRA_KMD[];
/** \endcond */


/**
 * 
 */
static KiSKernelModuleInfo const *KiInternal_GetKernelModuleInfoSectionStart(KiTVoid) {
    return (KiSKernelModuleInfo const *)__start_KIRA_KMD;
}

/**
 * 
 */
static KiSKernelModuleInfo const *KiInternal_GetKernelModuleInfoSectionEnd(KiTVoid) {
    return (KiSKernelModuleInfo const *)__stop_KIRA_KMD;
}
#else
    #error You need to implement the section start and -end symbols for this compiler.
#endif


/**
 * 
 */
static KiTSize KI_CALL KiInternal_GetKernelModuleInfoSectionSize(KiTVoid) {
    /*
     * This returns the total size of the section in bytes. This is absolutely not guaranteed to be the size of all the
     * structs placed in there; there might be padding before, after, and in between each struct.
     */
    return (KiTIntptr)KiInternal_GetKernelModuleInfoSectionEnd() - (KiTIntptr)KiInternal_GetKernelModuleInfoSectionStart();
}

/**
 *
 */
static KiSKernelModuleInfo const *KiInternal_YieldNextKernelModule(KiSKernelModuleInfo const *currModPtr) {
    if (currModPtr == nullptr)
        return nullptr;

    /* (1) Calculate the remaining size that we can search for more module info entries. */
    KiTSize remSizeInBytes  = KiInternal_GetKernelModuleInfoSectionSize();
            remSizeInBytes -= (KiTSize)((KiTByte *)currModPtr - (KiTByte *)KiInternal_GetKernelModuleInfoSectionStart());

    /* (2) Go from the current pointer and search for the next module info magic. */
    KiTByte const magicSig[16] = { '_', '_', 'A', 'n', 'a', 'r', 'c', 'h', 'i', 'a', 'M', 'a', 'm', 'a', '_', '_' };
    {
        /*
         * (3..) Try to find the next signature. As long as we have space for at least one entry in the section, we keep
         *       going. As the signature is smaller than the entire entry (due to being part of the full entry), we do
         *       not need to take any more security measures for the memcmp() call.
         */
        for (KiTByte const *currPtr = (KiTByte const *)currModPtr; remSizeInBytes >= sizeof *currModPtr;) {
            if (memcmp(currPtr, magicSig, sizeof magicSig) != 0) {
                /* (3.1) Didn't find it. Advance by one byte and keep going. */
                ++currPtr;
                --remSizeInBytes;

                continue;
            }

            /* (3.2) Found it! Yay! */
            return (KiSKernelModuleInfo const *)currPtr;
        }
    }

    /* Could not find another entry. */
    return nullptr;
}

/**
 *
 */
static KiSKernelModuleInfo const *KiInternal_BeginKernelModuleEnumeration(KiTVoid) {
    return KiInternal_YieldNextKernelModule(KiInternal_GetKernelModuleInfoSectionStart());
}

/**
 *
 */
static void KiInternal_EndKernelModuleEnumeration(KiTVoid) {
    KI_NOOP;
}


//KI_KRNLMOD_IMPORT(Registry);
//KI_KRNLMOD_IMPORT(ErrorStringificationService);
//KI_KRNLMOD_IMPORT(PoolAllocator);
//KI_KRNLMOD_IMPORT(RuntimeConfiguration);
//KI_KRNLMOD_IMPORT(ExceptionHandlingSystem);
//KI_KRNLMOD_IMPORT(DebugModuleControl);
//KI_KRNLMOD_IMPORT(ProfileManager);


/**
 */
static KiSKernelModuleInfo const *const gl_c_ModuleInitTable[] = {
    nullptr
    //&KI_KRNLMOD_IDENTIFY(RuntimeConfiguration),
    //&KI_KRNLMOD_IDENTIFY(ErrorStringificationService),
    //&KI_KRNLMOD_IDENTIFY(ProfileManager),
    //&KI_KRNLMOD_IDENTIFY(ExceptionHandlingSystem),
    //&KI_KRNLMOD_IDENTIFY(DebugModuleControl),
    //&KI_KRNLMOD_IDENTIFY(Registry),
    //&KI_KRNLMOD_IDENTIFY(PoolAllocator)
};

/**
 */
static KiTIndex gl_InitIndex = -1;
/** \endcond */


KiEErrorCode KI_CALL KiStartKernelModules(KiTVoid) {
    KiSKernelModuleInfo const *start = KiInternal_BeginKernelModuleEnumeration();
    {
        printf("Found these kernel modules:\n");
        for (; start != nullptr;) {
            printf("    %s\n", start->m_moduleInfo.mp_modId->mp_strPtr);

            KiSStaticArray const *x = start->mp_fnGetDeps();
            KiSUuid u = *((KiSUuid const **)x->mp_arrPtr)[0];

            start = KiInternal_YieldNextKernelModule((KiSKernelModuleInfo const *)((KiTByte *)start + 1));
        }
    }
    KiInternal_EndKernelModuleEnumeration();

    for (KiTIndex i = 0; i < (KiTIndex)KI_COUNTOF(gl_c_ModuleInitTable); i++) {
        KiSKernelModuleInfo const *const currModEntry = gl_c_ModuleInitTable[i];
        if (currModEntry == nullptr)
            continue;

        /* Initialize the module. */
        KiEErrorCode errCode = (*currModEntry->m_moduleInfo.mp_fnInit)(nullptr);
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
        KiSKernelModuleInfo const *const currModEntry = gl_c_ModuleInitTable[i];
        if (currModEntry == nullptr)
            continue;

        (*currModEntry->m_moduleInfo.mp_fnUninit)(nullptr);
    }

    /* "Perfectly clean... as all things should be." */
    gl_InitIndex = -1;
    return KiErr_Ok;
}
/** \endcond */


