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
 * \file  config.c
 * \brief implements the kernel-level configuration manager
 */


/* stdlib includes */
#if (!defined _CRT_SECURE_NO_WARNINGS)
    #define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdlib.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/json.h>

#include <kira/kernel/int/string.h>
#include <kira/kernel/int/krnlmod.h>
#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef struct KiSConfigurationManagementState {
    KiSJson *mp_config;
    KiSJson *mp_profile;
} KiSConfigurationManagementState;


/**
 */
static KiSConfigurationManagementState gl_ConfigurationManagement = { 0 };

/**
 */
static KiSStringView const gl_c_DefaultConfigDirPaths[] = {
    KI_MAKE_STRING_VIEW("cfg"),
    KI_MAKE_STRING_VIEW("conf"),
    KI_MAKE_STRING_VIEW("config"),
    KI_MAKE_STRING_VIEW("configuration")
};


/**
 */
static KiSString *KI_CALL KiInternal_BuildAbsolutePath(KiTChar const *pathStr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);

    KiTBool isRel;
    KiSString *resPtr;
    KiEErrorCode errCode;
    {
        errCode = (isRel = KiPlatform_IsPathRelative(pathStr)) == KI_TRUE
            ? KiCreateStringApplicationRootDir(&resPtr)
            : KiCreateString(pathStr, &resPtr)
        ;
        if (errCode != KiErr_Ok)
            return nullptr;
        
        if (isRel == KI_TRUE && KiPushPathComponent(resPtr, '/', pathStr) != KiErr_Ok) {
            KiDestroyString(resPtr);

            resPtr = nullptr;
        }
    }

    return resPtr;
}

/**
 */
static KiTBool KI_CALL KiInternal_CheckConfigurationPathCandidate(KiSString *pathCandid) {
    KI_ASSERT(pathCandid != nullptr, KiErr_InParameter);

    /* (1) Check if the candidate path even exists and is a directory. */
    KiTBool res = KiPlatform_PathExists(KiGetCString(pathCandid), KI_TRUE);

    /* (2.a) Check if './init.json' exists. */
    if (res == KI_TRUE) {
        if (KiPushPathComponent(pathCandid, '/', "init.json") != KiErr_Ok)
            return KI_FALSE;

        res = KiPlatform_PathExists(KiGetCString(pathCandid), KI_FALSE);
        KiPopPathComponent(pathCandid, '/');
    }
    /* (2.b) Check if './profiles' exists. */
    if (res == KI_TRUE) {
        if (KiPushPathComponent(pathCandid, '/', "profiles") != KiErr_Ok)
            return KI_FALSE;

        res = KiPlatform_PathExists(KiGetCString(pathCandid), KI_TRUE);
        KiPopPathComponent(pathCandid, '/');
    }

    return res;
}

/**
 */
static KiSString *KI_CALL KiInternal_DetermineConfigDirectoryPath(KiTVoid) {
    /* First of all, we build an ordered list of configuration directory candidates. */
    KiTChar const *configSearchPaths[KI_COUNTOF(gl_c_DefaultConfigDirPaths) + 1] = { nullptr };
    for (KiTSize i = 0; i < KI_COUNTOF(configSearchPaths); i++) {
        if (i == 0) {
            configSearchPaths[0] = getenv("KIRA_CONFIGURATION_DIRECTORY");

            continue;
        }

        configSearchPaths[i] = gl_c_DefaultConfigDirPaths[i - 1].mp_strPtr;
    }

    /* Then, we check for each path if
     *  (1) it exists
     *  (2) if it contains
     *   (a) an 'init.json' file
     *   (b) a 'profiles' directory
     *
     * We select the first path candidate that satisfies the above conditions.
     */
    KiSString *currPath;
    KiEErrorCode errCode;
    {
        for (KiTSize i = 0; i < KI_COUNTOF(configSearchPaths); i++) {
            if (configSearchPaths[i] == nullptr)
                continue;

            /* Build absolute path. */
            currPath = KiInternal_BuildAbsolutePath(configSearchPaths[i]);
            if (currPath == nullptr)
                return nullptr;

            if (KiInternal_CheckConfigurationPathCandidate(currPath) == KI_TRUE) {
                /* Found a valid directory. Use it. */
                return currPath;
            }
            KiDestroyString(currPath);
        }
    }

    /* Could not find a valid config directory. */
    return nullptr;
}


/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(ConfigurationManagement)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    KiSString *cfgPath = KiInternal_DetermineConfigDirectoryPath();
    {

    }

    /* Stub. */
    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(ConfigurationManagement)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    return KiErr_Ok;
}
/** \endcond */


/** \cond */
KI_KRNLMOD_DEFINE(ConfigurationManagement) {
    .m_structSize = sizeof(KiSModuleInfo),
    .mp_modUuid   = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId     = &KI_MAKE_STRING_VIEW("configuration manager"),
    .m_modFlags   = 0,

    .mp_fnInit    = &KI_KRNLMOD_INITFN(ConfigurationManagement),
    .mp_fnUninit  = &KI_KRNLMOD_UNINITFN(ConfigurationManagement)
};
/** \endcond */


