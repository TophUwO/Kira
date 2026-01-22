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
#include <kira/kernel/int/fenum.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiStartKernelModules(KiTVoid);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiShutdownKernelModules(KiTVoid);


/**
 */
KI_NATIVE typedef struct KiSKernelState {
    KiSReturnState           m_retState;
    KiSJson                 *mp_rtConfig;
    KiSRuntimeSpecification  m_rtSpecs;
} KiSKernelState;
/**
 */
static KiSKernelState gl_KernelState = { 0 };


/**
 */
static KiSString *KI_CALL KiInternal_GetApplicationRootDirectoryAsString(KiTVoid) { 
    KiSString *resPtr = nullptr;
    {
        /* Create string. */
        KiEErrorCode errCode = KiCreateString(nullptr, &resPtr);
        if (errCode != KiErr_Ok)
            return nullptr;

        /* Get runtime directory. */
        KiTChar const *rootDir = KiPlatform_GetApplicationRootDirectory();
        {
            if (rootDir == nullptr) {
                KiDestroyString(resPtr);

                return nullptr;
            }

            /* Copy into mutable string. */
            if ((errCode = KiAssignToString(resPtr, rootDir)) != KiErr_Ok) {
                KiDestroyString(resPtr);

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
    KiSString *configFilePath = KiInternal_GetApplicationRootDirectoryAsString();
    {
        if (configFilePath == nullptr)
            return KiErr_GetSystemPath;

        /* Push file name. */
        KiEErrorCode errCode = KiPushPathComponent(configFilePath, '/', "launch.json");
        if (errCode != KiErr_Ok) {
            KiDestroyString(configFilePath);

            return errCode;
        }

        /* Load JSON. */
        gl_KernelState.mp_rtConfig = KiOpenJsonDocument(KiGetCString(configFilePath));
    }
    KiDestroyString(configFilePath);

    /* If the JSON could be loaded, we are good else we riot. */
    return gl_KernelState.mp_rtConfig != nullptr ? KiErr_Ok : KiErr_LoadJsonDocument;
}

/**
 */
static KiSString *KI_CALL KiInternal_KrnlPickProfile(KiTVoid) {
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
    KiSString *profileDir = KiInternal_GetApplicationRootDirectoryAsString();
    {
        if (profileDir != nullptr)
            return nullptr;

        /* Append relative profile path. */
        KiEErrorCode errCode = KiPushPathComponent(profileDir, '/', attrQuery[0].mp_strValue);
        if (errCode != KiErr_Ok) {
            KiDestroyString(profileDir);

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
    /** \todo IMPLEMENT PICKING PROFILE */
}
/** \endcond */


KiEErrorCode KI_CALL KiStartup(KiSRuntimeSpecification const *rtSpecs) {
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
            .m_isAssertsEnabled = KI_TRUE
        };

        KiEErrorCode errCode = KiStartDebugSession(
            rtSpecs->mp_dbgOpts != nullptr
                ? rtSpecs->mp_dbgOpts
                : (gl_KernelState.m_rtSpecs.mp_dbgOpts = (KiSDebugOptions *)&gl_c_DefDbgOptions)
        );
        if (errCode != KiErr_Ok)
            return errCode;
    }
#endif

    /* Startup kernel. */
    if ((errCode = KiStartKernelModules()) != KiErr_Ok)
        return errCode;

    /* Load profile in config or pick a suitable one. */
    KiSString *pickedProfilePath = KiInternal_KrnlPickProfile();
    {
        if (pickedProfilePath == nullptr)
            return KiErr_PickProfile;

        /* Set it as active profile. */
        errCode = KiLoadProfile(KiGetCString(pickedProfilePath));
        if (errCode != KiErr_Ok) {
            KiDestroyString(pickedProfilePath);

            return errCode;
        }
    }
    KiDestroyString(pickedProfilePath);

    // enum all modules 
    // add all comps to reg
    // create all debug handlers
    // load the rest of the modules (via iterative dfs)

    /* All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiShutdown(KiTVoid) {
    /* Unload profile. */
    KiUnloadProfile();
    /* Shutdown kernel. */
    KiEErrorCode const errCode = KiShutdownKernelModules();

    /* Shutdown debug session. */
#if (defined KI_CONFIG_DEBUG)
    KiStopDebugSession();
#endif

    /* Unload runtime config. */
    KiCloseJsonDocument(gl_KernelState.mp_rtConfig);
    return errCode;
}

KiTVoid KI_CALL KiRun(KiSReturnState *retStatePtr) {
    /* Locate client and invoke Run(). */

    /* At last, copy runtime return state. */
    KiTSize const dstSize = retStatePtr->m_structSize;
    {
        memcpy_s(retStatePtr, dstSize, &gl_KernelState.m_retState, gl_KernelState.m_retState.m_structSize);

        retStatePtr->m_structSize = dstSize;
    }
}

KiTVoid KI_CALL KiQuit(KiEErrorCode errCode) {
    KiSReturnState *const krnlRetState = &gl_KernelState.m_retState;

    *krnlRetState = (KiSReturnState){
        .m_structSize   = sizeof *krnlRetState,
        .m_errCode      = errCode,
        .m_isRunning    = KI_FALSE,
        .m_wantsRestart = KI_FALSE
    };
}

KiTVoid KI_CALL KiRestart(KiEErrorCode errCode) {
    KiSReturnState *const krnlRetState = &gl_KernelState.m_retState;

    *krnlRetState = (KiSReturnState){
        .m_structSize   = sizeof *krnlRetState,
        .m_errCode      = errCode,
        .m_isRunning    = KI_FALSE,
        .m_wantsRestart = KI_TRUE
    };
}


KiSReturnState *KI_CALL KiGetReturnState(KiTVoid) {
    return (KiSReturnState *)&gl_KernelState.m_retState;
}

KiSRuntimeSpecification const *KI_CALL KiGetRuntimeSpecification(KiTVoid) {
    return (KiSRuntimeSpecification const *)&gl_KernelState.m_rtSpecs;
}

KiSJson const *KI_CALL KiGetRuntimeConfiguration(KiTVoid) {
    return (KiSJson const *)gl_KernelState.mp_rtConfig;
}


KiTVoid KI_CALL KiInvokeOnAssertHandler(KiTVoid const *extraParam) {
    
}


