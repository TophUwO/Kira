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
 * \file  kernel.c
 * \brief implements the kernel startup- and shutdown routines
 */


/* stdlib includes */
#include <stdatomic.h>

#include <string.h>

/* Kira includes */
#include <kira/kernel/rt.h>
#include <kira/kernel/json.h>
#include <kira/kernel/profile.h>

#include <kira/kernel/int/string.h>
#include <kira/kernel/int/platform.h>
#include <kira/kernel/int/krnlboot.h>
#include <kira/kernel/int/fenum.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef struct KiSKernelState {
    KiTAtomicFlag            m_isQuit;
    KiSReturnState           m_retState;
    KiSJson                 *mp_rtConfig;
    KiSRuntimeSpecification  m_rtSpecs;
} KiSKernelState;
/**
 */
static KiSKernelState gl_KernelState = {};


/**
 */
static KiSKrnlString *KI_CALL KiInternal_GetApplicationRootDirectoryAsString(KiTVoid) { 
    KiSKrnlString *resPtr;
    {
        /* Create string. */
        KiEErrorCode errCode = KiKrnlStringCreate(nullptr, &resPtr);
        if (errCode != KiErr_Ok)
            return nullptr;

        /* Get runtime directory. */
        KiTChar const *rootDir = KiPlatform_GetApplicationRootDirectory();
        {
            if (rootDir == nullptr) {
                KiKrnlStringDestroy(resPtr);

                return nullptr;
            }

            /* Copy into mutable string. */
            if ((errCode = KiKrnlStringAssign(resPtr, rootDir)) != KiErr_Ok) {
                KiKrnlStringDestroy(resPtr);

                KiPlatform_FreeString((KiTChar *)rootDir);
                return nullptr;
            }
        }
        KiPlatform_FreeString((KiTChar *)rootDir);
    }

    /* All good. */
    return resPtr;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlLoadRtConfig(KiTVoid) {
    KiSKrnlString *configFilePath = KiInternal_GetApplicationRootDirectoryAsString();
    {
        if (configFilePath != nullptr)
            return KiErr_GetSystemPath;

        /* Push file name. */
        KiEErrorCode errCode = KiKrnlStringPushPathComponent(configFilePath, "launch.json");
        if (errCode != KiErr_Ok) {
            KiKrnlStringDestroy(configFilePath);

            return errCode;
        }

        /* Load JSON. */
        gl_KernelState.mp_rtConfig = KiOpenJsonDocument(KiKrnlStringCStr(configFilePath));
    }
    KiKrnlStringDestroy(configFilePath);

    /* If the JSON could be loaded, we are good else we riot. */
    return gl_KernelState.mp_rtConfig != nullptr ? KiErr_Ok : KiErr_LoadJsonDocument;
}

/**
 */
static KiSKrnlString *KI_CALL KiInternal_KrnlPickProfile(KiTVoid) {
    /* Get all the attributes we might need. */
    KiSJsonValueQuery attrQuery[] = {
        [0] = { "/profilePath",          KiJsonValTy_String  },
        [1] = { "/fallbackToDefProfile", KiJsonValTy_Boolean },
        [2] = { "/preferredProfile",     KiJsonValTy_String  },
        [3] = { "/defaultProfile",       KiJsonValTy_String  }
    };
    KiTBool isError = KiGetJsonElementValues(gl_KernelState.mp_rtConfig, attrQuery, KI_COUNTOF(attrQuery));
    if (isError)
        return nullptr;

    /* (1) Get the root directory for profile picking. */
    KiSKrnlString *profileDir = KiInternal_GetApplicationRootDirectoryAsString();
    {
        if (profileDir != nullptr)
            return nullptr;

        /* Append relative profile path. */
        KiEErrorCode errCode = KiKrnlStringPushPathComponent(profileDir, attrQuery[0].mp_strValue);
        if (errCode != KiErr_Ok) {
            KiKrnlStringDestroy(profileDir);

            return nullptr;
        }
    }
    
    /*
     * (3) Iterate over the profile directory and do two things:
     *  (a) If we find the profile we want, store file name and return.
     *  (b) If we find the default profile, save it.
     *
     * Then, after we are done iterating, return the significant profile (i.e., either the one we wanted, or, if not
     * found and fallback is enabled, the default one.) If neither the one we wanted nor the default profile could be
     * located, we return nullptr. The caller will be able to handle this appropriately.
     */
    KiSFileEnumerationContext *enumCtxt = KiFileEnumerationContextCreate(&(KiSFileEnumerationProperties){
        .m_structSize     = sizeof(KiSFileEnumerationProperties),
        .m_nSkipRes       = 0,
        .m_nMaxCount      = -1,
        .m_isRecursive    = KI_TRUE,
        .m_doWalkSymlinks = KI_FALSE,
        .mp_rootDir       = KiKrnlStringCStr(profileDir)
    });
    KiSFileEnumerationResult *currRes;
    {
        KiTBool gotRes = KI_FALSE;
        while (!gotRes && (currRes = KiFileEnumerationContextYield(enumCtxt)) != nullptr) {
            if (!strcmp(attrQuery[2].mp_strValue, currRes->m_fileName.mp_strPtr)) {

                gotRes = KI_TRUE;
            } else if (attrQuery[1].m_boolValue && !strcmp(attrQuery[3].mp_strValue, currRes->m_fileName.mp_strPtr)) {
                /* Found default profile. */
                
            }

            KiFileEnumerationContextDiscard(enumCtxt);
        }
    }
    KiFileEnumerationContextDestroy(enumCtxt);
}
/** \endcond */


KiEErrorCode KI_CALL KiKrnlStartup(KiSRuntimeSpecification const *rtSpecs) {
    if (rtSpecs == nullptr)
        return KiErr_InParameter;

    /* Clear kernel state. This is to be done in case of a restart via KiKrnlRestart(). */
    memset(&gl_KernelState, 0, sizeof gl_KernelState);
    /* Copy runtime specification. */
    KiTSize size2Copy = KI_MIN(sizeof gl_KernelState.m_rtSpecs, rtSpecs->m_structSize);
    {
        memcpy((KiTVoid *)&gl_KernelState.m_rtSpecs, (KiTVoid const *)rtSpecs, size2Copy);

        gl_KernelState.m_rtSpecs.m_structSize = size2Copy;
    }

    /* Load runtime config. */
    KiEErrorCode errCode = KiInternal_KrnlLoadRtConfig();
    if (errCode != KiErr_Ok)
        return errCode;

    /* Start debugging session if necessary. */
#if (defined KI_CONFIG_DEBUG)
    {
        /**
         * \brief represents the default debug options in case no overriding debug options were passed by the launcher
         *        component
         */
        static KiSDebugOptions constexpr gl_c_DefDbgOptions = {
            .m_structSize       = sizeof gl_c_DefDbgOptions,
            .m_useDetRng        = KI_TRUE,
            .m_isAssertsEnabled = KI_TRUE,
            .m_isBreakOnAssert  = KI_FALSE
        };

        KiEErrorCode errCode = KiDbgStartSession(
            rtSpecs->mp_dbgOpts != nullptr
                ? rtSpecs->mp_dbgOpts
                : (gl_KernelState.m_rtSpecs.mp_dbgOpts = (KiSDebugOptions *)&gl_c_DefDbgOptions)
        );
        if (errCode != KiErr_Ok)
            return errCode;
    }
#endif

    /* Startup kernel. */
    if ((errCode = KiKrnlStartSystems()) != KiErr_Ok)
        return errCode;

    /* Load profile in config or pick a suitable one. */
    KiSKrnlString *pickedProfilePath = KiInternal_KrnlPickProfile();
    {
        if (pickedProfilePath == nullptr)
            return KiErr_PickProfile;

        /* Set it as active profile. */
        errCode = KiLoadProfile(KiKrnlStringCStr(pickedProfilePath));
        if (errCode != KiErr_Ok) {
            KiKrnlStringDestroy(pickedProfilePath);

            return errCode;
        }
    }
    KiKrnlStringDestroy(pickedProfilePath);

    // enum all modules 
    // add all comps to reg
    // create all debug handlers
    // load the rest of the modules (via iterative dfs)

    /* All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiKrnlShutdown(KiTVoid) {
    /* Unload profile. */
    KiUnloadProfile();
    /* Shutdown kernel. */
    KiEErrorCode const errCode = KiKrnlShutdownSystems();

    /* Shutdown debug session. */
#if (defined KI_CONFIG_DEBUG)
    KiDbgStopSession();
#endif

    /* Unload runtime config. */
    KiCloseJsonDocument(gl_KernelState.mp_rtConfig);
    return errCode;
}

KiTVoid KI_CALL KiKrnlRun(KiSReturnState *retStatePtr) {
    /* Run main loop. */
    while (atomic_fetch_or(&gl_KernelState.m_isQuit, KI_FALSE) == KI_FALSE) {
        
    }

    /* At last, copy runtime return state. */
    KiTSize const dstSize = retStatePtr->m_structSize;
    {
        memcpy_s(retStatePtr, dstSize, &gl_KernelState.m_retState, gl_KernelState.m_retState.m_structSize);

        retStatePtr->m_structSize = dstSize;
    }
}

KiTVoid KI_CALL KiKrnlQuit(KiEErrorCode errCode) {
    if (atomic_fetch_or(&gl_KernelState.m_isQuit, KI_TRUE) == KI_FALSE) {
        KiSReturnState *krnlRetState = &gl_KernelState.m_retState;

        *krnlRetState = (KiSReturnState){
            .m_structSize   = sizeof *krnlRetState,
            .m_errCode      = errCode,
            .m_wantsRestart = KI_FALSE
        };
    }
}

KiTVoid KI_CALL KiKrnlRestart(KiEErrorCode errCode) {
    if (atomic_fetch_or(&gl_KernelState.m_isQuit, KI_TRUE) == KI_FALSE) {
        KiSReturnState *krnlRetState = &gl_KernelState.m_retState;

        *krnlRetState = (KiSReturnState){
            .m_structSize   = sizeof *krnlRetState,
            .m_errCode      = errCode,
            .m_wantsRestart = KI_TRUE
        };
    }
}


KiSRuntimeSpecification const *KI_CALL KiKrnlGetRuntimeSpecification(KiTVoid) {
    return (KiSRuntimeSpecification const *)&gl_KernelState.m_rtSpecs;
}

KiSJson const *KI_CALL KiKrnlGetRuntimeConfig(KiTVoid) {
    return (KiSJson const *)gl_KernelState.mp_rtConfig;
}


KiTVoid KI_CALL KiKrnlInvokeOnAssertHandler(KiTVoid const *extraParam) {
    /* If we are in test mode, we throw. Otherwise, we invoke the debug assert handler, i.e., the */
}


