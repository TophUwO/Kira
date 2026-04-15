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
 * \file  config.c
 * \brief implements the kernel-level configuration manager
 */


/* stdlib includes */
#include <string.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/json.h>

#include <kira/kernel/int/config.h>
#include <kira/kernel/int/string.h>
#include <kira/kernel/int/krnlmod.h>
#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef struct KiSRuntimeConfigurationState {
    KiSJson   *mp_config;
    KiSString *mp_configRootDir;
} KiSRuntimeConfigurationState;

/**
 */
static KiSRuntimeConfigurationState gl_RuntimeConfigState = { nullptr };


/**
 */
static KiSStringView const *gl_c_DefaultConfigDirPaths[] = {
    &KI_MAKE_STRING_VIEW("conf")
};

/**
 */
static KiSStringView const *gl_c_ProfileExts[] = {
    &KI_MAKE_STRING_VIEW(".json"),
    &KI_MAKE_STRING_VIEW(".JSON")
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
static KiTBool KI_CALL KiInternal_FilenameHasJsonExt(KiTChar const *pathStr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);

    KiTChar const *lastDot = strrchr(pathStr, '.');
    {
        for (KiTSize i = 0; i < KI_COUNTOF(gl_c_ProfileExts); i++)
            if (!strcmp(lastDot, gl_c_ProfileExts[i]->mp_strPtr))
                return KI_TRUE;
    }

    return KI_FALSE;
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
    /*
     * First of all, we get the value of the potentially defined configuration directory override. The function will
     * automatically handle the cases in which this variable was not defined or it is empty.
     */
    KiTChar *cfgPathOverride = KiPlatform_GetEnvironmentVariable("KIRA_CONFIGURATION_DIRECTORY", KI_DONTCARE(KiTSize));

    /* Be build an ordered list of configuration directory candidates. */
    KiTChar const *configSearchPaths[KI_COUNTOF(gl_c_DefaultConfigDirPaths) + 1] = { nullptr };
    for (KiTSize i = 0; i < KI_COUNTOF(configSearchPaths); i++) {
        if (i == 0) {
            configSearchPaths[0] = cfgPathOverride;

            continue;
        }

        configSearchPaths[i] = gl_c_DefaultConfigDirPaths[i - 1]->mp_strPtr;
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
                break;

            if (KiInternal_CheckConfigurationPathCandidate(currPath) == KI_TRUE) {
                KiPlatform_FreeString(cfgPathOverride);

                /* Found a valid directory. Use it. */
                return currPath;
            }
            KiDestroyString(currPath);
        }
    }

    /* Could not find a valid config directory. */
    KiPlatform_FreeString(cfgPathOverride);
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

        gl_RuntimeConfigState.mp_config = KiOpenJsonDocument(KiGetCString(confPath), &errCode);
    }
    KiPopPathComponent(confPath, '/');

    return errCode;
}

/**
 * \ki_tested{tested;
 *  <b>The following test cases have been successfully verified:</b>
 *  <ul>
 *   \li assertion failures upon missing/incorrect preconditions
 *   \li adding a normal environment variable
 *   \li adding an empty environment variable
 *   \li unsetting an environment variable (via <tt>nullptr>-value)
 *   \li adding environment variable with non-Latin name and value
 *  </ul>;
 *  Mar 8, 2026
 * }
 */
static KiTVoid KI_CALL KiInternal_SetConfigEnvironmentVariables(KiSJson const *cfgDoc) {
    KI_ASSERT(cfgDoc != nullptr, KiErr_InParameter);

    /* (1) Get the environment block from the config (if present). */
    KiSJsonValueQuery envObjQuery = { .mp_pathStr = "/environment/env", .m_reqType = KiJsonValTy_Object };
    {
        KiTBool const queryRes = KiGetJsonElementValue(gl_RuntimeConfigState.mp_config, &envObjQuery);

        if (queryRes == KI_FALSE || envObjQuery.mp_objValue == nullptr)
            return;
    }

    /*
     * (2) Go through the block (JSON dictionary) and add all the environment variables. The key is the variable name
     *     and the value is the value of that environment variable.
     */
    for (KiSJson *var = (KiSJson *)envObjQuery.mp_objValue; var != nullptr; var = KiGetNextJsonElement(var)) {
        /* Get the key. */
        KiTChar const *varName = KiGetJsonElementName(var);
        if (varName == nullptr || *varName == '\0')
            continue;

        /* Get the value associated with the current key. */
        KiSJsonValueQuery varValueQuery = { .mp_pathStr = nullptr, .m_reqType = KiJsonValTy_StrOrNull };
        {
            KiTBool const queryRes = KiGetJsonElementValue(var, &varValueQuery);

            if (queryRes == KI_FALSE)
                continue;
        }

        KiPlatform_SetEnvironmentVariable(varName, varValueQuery.mp_strValue);
    }
}


/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(RuntimeConfiguration)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /* (0) Reset state from a potential previous session. */
    memset(&gl_RuntimeConfigState, 0, sizeof gl_RuntimeConfigState);

    /* (1) Determine configuration directory path. */
    gl_RuntimeConfigState.mp_configRootDir = KiInternal_DetermineConfigDirectoryPath();
    {
        if (gl_RuntimeConfigState.mp_configRootDir == nullptr)
            return KiErr_NoConfigDirectory;

        /* (2) Load config file as JSON. */
        KiEErrorCode errCode = KiInternal_LoadInitFile(gl_RuntimeConfigState.mp_configRootDir);
        if (errCode != KiErr_Ok) {
            KiDestroyString(gl_RuntimeConfigState.mp_configRootDir);

            return errCode;
        }

        /* (3) Add all environment variables listed in the config's "environment" section. */
        KiInternal_SetConfigEnvironmentVariables(gl_RuntimeConfigState.mp_config);
    }

    return KiErr_Ok;
}

/**
 * \ki_tested
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(RuntimeConfiguration)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    KiCloseJsonDocument(gl_RuntimeConfigState.mp_config);
    KiDestroyString(gl_RuntimeConfigState.mp_configRootDir);

    return KiErr_Ok;
}
/** \endcond */


/**
 * \ki_tested{tested;
 *  <b>The following test cases have been successfully verified:</b>
 *  <ul>
 *   \li assertion failures upon missing/incorrect preconditions
 *   \li correct path formatting
 *  </ul>;
 *  Mar 7, 2026
 * }
 */
KiSString *KI_CALL KiGetRootProfilePath(KiTVoid) {
    KI_ASSERT(gl_RuntimeConfigState.mp_config != nullptr,        KiErr_InParameter);
    KI_ASSERT(gl_RuntimeConfigState.mp_configRootDir != nullptr, KiErr_InParameter);

    KiSString *res;
    {
        if (KiDuplicateString(gl_RuntimeConfigState.mp_configRootDir, &res) != KiErr_Ok)
            return nullptr;

        /*
         * Get root profile file name (without extension). Retrieving this value and using it is safe because the
         * runtime config is considered read-only and thus the queried values will never change throughout the
         * application's runtime.
         */
        KiSJsonValueQuery rootProfileQuery = { .mp_pathStr = "/activeProfile", .m_reqType  = KiJsonValTy_String };
        if (KiGetJsonElementValue(gl_RuntimeConfigState.mp_config, &rootProfileQuery) == KI_FALSE) {
            KiDestroyString(res);

            return nullptr;
        }

        /* Add file name to root dir. */
        KiEErrorCode errCode = KiErr_Ok;
        {
            KiTChar const pathSep = KiPlatform_GetPathSeparator();

            (KiTVoid)(
                   (errCode = KiPushPathComponent(res, pathSep, "profiles"))                   == KiErr_Ok
                && (errCode = KiPushPathComponent(res, pathSep, rootProfileQuery.mp_strValue)) == KiErr_Ok
                && (errCode = KiPushPathComponent(res, '.', "json"))                           == KiErr_Ok
            );

            if (errCode != KiErr_Ok) {
                KiDestroyString(res);

                return nullptr;
            }
        }
    }

    return res;
}


/** \cond */
KI_KRNLMOD(RuntimeConfiguration, {
    &KI_MAKE_UUID(0xFFFFFFFF, 0xAAAA, 0xBBBB, 0xCCCCCCCCCCCCCCCC)
}, {
    .mp_modUuid  = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId    = &KI_MAKE_STRING_VIEW("runtime configuration"),
    .m_modFlags  = 0
});
/** \endcond */


