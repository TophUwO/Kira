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
/** \cond */
#if (!defined _CRT_SECURE_NO_WARNINGS)
    #define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdlib.h>
/** \endcond */

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/json.h>

#include <kira/kernel/int/string.h>
#include <kira/kernel/int/buffer.h>
#include <kira/kernel/int/krnlmod.h>
#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef struct KiSRuntimeConfigurationState {
    KiSJson        *mp_config;
    KiSBuffer      *mp_cmdl;
    KiSBuffer      *mp_envVarStorage;
    KiSStringView **mpp_envVarViews;
    KiTSize         m_nEnvVars;
    KiSJson        *mpp_preldProfiles;
    KiTSize         m_nPreldProfiles;
} KiSRuntimeConfigurationState;


/**
 */
static KiSRuntimeConfigurationState gl_Config = { 0 };

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
     * We select the first path candidate that satisfies all of the above conditions.
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
 * \brief  loads the initialization file, usually "init.json" located in the root of the configuration directory
 * \param  [in, out] confPath modifiable path to the root of the (configuration) directory to load the init config from
 * \return \c KiErr_Ok on success, a value other than \c KiErr_Ok on failure. The following error codes can be returned
 *         by this function:
 *          <table>
 *           <tr><th> Identifier               <th> Reason
 *           <tr><td> KiErr_Ok                 <td> no error
 *           <tr><td> KiErr_ContainerFull      <td> operation would have exceeded the container's maximum capacity
 *           <tr><td> KiErr_MemoryReallocation <td> operation caused a memory reallocation which failed
 *           <tr><td> KiErr_LoadJsonDocument   <td> initialization configuration file could not be loaded
 *          </table>
 * \note    This function may allocate memory using <tt>[m|c|re]alloc()]</tt> if needed.
 * \warning The behavior is undefined if \c confPath is \c NULL or if it points to an invalid memory location.
 */
static KiEErrorCode KI_CALL KiInternal_LoadInitFile(KiSString *confPath) {
    KI_ASSERT(confPath != nullptr, KiErr_InOutParameter);

    KiEErrorCode errCode = KiPushPathComponent(confPath, '/', "init.json");
    {
        if (errCode != KiErr_Ok)
            return errCode;

        if ((gl_Config.mp_config = KiOpenJsonDocument(KiGetCString(confPath))) == nullptr)
            errCode = KiErr_LoadJsonDocument;
    }
    KiPopPathComponent(confPath, '/');

    return errCode;
}

/**
 */
static KiTVoid KI_CALL KiInternal_UnloadInitFile(KiTVoid) {
    KiCloseJsonDocument(gl_Config.mp_config);
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CreateEnvironmentAndCommandline(KiTVoid) {
    KiTSize bufSize;
    KiTChar *rawCmdl = KiPlatform_GetCommandLine(&bufSize);
    {
        if (rawCmdl == nullptr)
            return KiErr_SysGetCommandLine;

        KiEErrorCode errCode = KiCreateBufferFromExisting(bufSize, rawCmdl, &gl_Config.mp_cmdl);
        if (errCode != KiErr_Ok) {
            KiPlatform_FreeString(rawCmdl);

            return errCode;
        }
    }

    return KiErr_Ok;
}


/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(ConfigurationManagement)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    KiSString *cfgPath = KiInternal_DetermineConfigDirectoryPath();
    {
        if (cfgPath == nullptr)
            return KiErr_NoConfigDirectory;

        /* Load the init.cfg file. */
        KiEErrorCode errCode = KiInternal_LoadInitFile(cfgPath);
        if (errCode != KiErr_Ok) {
            KiDestroyString(cfgPath);

            return errCode;
        }
    }
    KiDestroyString(cfgPath);

    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(ConfigurationManagement)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    KiCloseJsonDocument(gl_Config.mp_config);

    return KiErr_Ok;
}
/** \endcond */


KiSStringView KI_CALL KiGetCommandLine(KiTVoid) {
    KI_ASSERT(gl_Config.mp_cmdl != nullptr, KiErr_InParameter);

    KiTSize const size = KiGetBufferPosition(gl_Config.mp_cmdl);
    {
        if (size < sizeof "\0")
            return (KiSStringView){ .mp_strPtr = nullptr, .m_sizeInBytes = 0 };

        return (KiSStringView){
            .mp_strPtr     = KiGetBufferPointer(gl_Config.mp_cmdl, 0),
            .m_sizeInBytes = KiGetBufferPosition(gl_Config.mp_cmdl) - sizeof "\0"
        };
    }
}

KiSStringView const **KI_CALL KiGetEnvironmentVariables(KiTVoid) {

}


KiSJson const *KI_CALL KiGetPreloadedProfiles(KiTSize *nProfilesPtr) {

}


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


