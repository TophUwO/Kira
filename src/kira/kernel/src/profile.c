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
 * \file  profile.c
 * \brief implements the profile management module
 */


/* stdlib includes */
#include <string.h>

/* Kira includes */
#include <kira/kernel/json.h>

#include <kira/kernel/int/config.h>
#include <kira/kernel/int/krnlmod.h>
#include <kira/kernel/int/platform.h>
#include <kira/kernel/int/array.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef enum KiEProfileLoadingPolicy {
    KiProfLdPol_Unknown = 0,

    KiProfLdPol_Load    = 1,
    KiProfLdPol_Skip    = 2,
    KiProfLdPol_Ignore  = 3,

    KI_ENUM_COUNT(KiProfLdPol)
} KiEProfileLoadingPolicy;


/**
 */
KI_NATIVE typedef struct KiSProfileManagerState {
    KiSArray *mp_profiles;
} KiSProfileManagerState;

/**
 */
static KiSProfileManagerState gl_ProfileManState = { nullptr };


/**
 */
static KiTVoid KI_CALL KiInternal_UnloadAllProfiles(KiSArray *profiles) {
    if (profiles == nullptr)
        return;

    KiTSize nProfiles;
    KiTVoid **mappedProfArr = KiMapArray(profiles, 0, KiGetArrayElementCount(profiles), &mappedProfArr, &nProfiles);
    {
        for (KiTSize i = 0; i < nProfiles; i++)
            KiCloseJsonDocument(mappedProfArr[i]);
    }

    KiClearArray(profiles);
}

/**
 */
static KiEProfileLoadingPolicy KI_CALL KiInternal_GetProfileVisibilityId(KiTChar const *ldPolStr) {
    KI_ASSERT(ldPolStr != nullptr, KiErr_InParameter);

    if      (!strcmp(ldPolStr, "load"))   return KiProfLdPol_Load;
    else if (!strcmp(ldPolStr, "ignore")) return KiProfLdPol_Ignore;
    else if (!strcmp(ldPolStr, "skip"))   return KiProfLdPol_Skip;

    return KiProfLdPol_Unknown;
}


static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(ProfileManager)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /* (0) Reset state from a potential previous session. */
    memset(&gl_ProfileManState, 0, sizeof gl_ProfileManState);

    /* (1) Get path of root profile. */
    KiSArray  *profArr  = nullptr;
    KiSString *profPath = KiGetRootProfilePath();
    {
        if (profPath == nullptr)
            return KiErr_NoProfileToLoad;

        /* (2) Create profile array. */
        KiEErrorCode errCode = KiCreateArray(&profArr);
        if (errCode != KiErr_Ok) {
            KiDestroyString(profPath);

            return errCode;
        }

        /* (3..) Load the profiles in order. */
        KiSJson *currProf       = nullptr;
        KiTBool  hasBaseProfile = KI_FALSE;
        do {
            /* (4) Load profile. */
            if ((currProf = KiOpenJsonDocument(KiGetCString(profPath), &errCode)) == nullptr) {
                KiInternal_UnloadAllProfiles(profArr);
                KiDestroyArray(profArr);

                KiDestroyString(profPath);
                return errCode;
            }

            /*
             * (6.1) See if the current profile's "baseProfile" property is set. If yes, adjust our current profile path
             *       so that in the next iteration it can be loaded. Otherwise, we are finished.
             * (6.2) Check profile visibility. This determines if the profile is even to be considered. 
             */
            KiSJsonValueQuery baseProfQuery[2] = {
                { .mp_pathStr = "/config/baseProfile",   .m_reqType = KiJsonValTy_StrOrNull, .m_isOpt = KI_TRUE }, /* idx 0 */
                { .mp_pathStr = "/config/loadingPolicy", .m_reqType = KiJsonValTy_String,    .m_isOpt = KI_TRUE }  /* idx 1 */
            };
            {
                KiTBool const queryRes  = KiGetJsonElementValues(currProf, baseProfQuery, KI_COUNTOF(baseProfQuery));
                KiTBool const foundBase = baseProfQuery[0].m_errCode != KiErr_JsonAttribNotFound;
                KiTBool const foundPol  = baseProfQuery[1].m_errCode != KiErr_JsonAttribNotFound;

                if (foundBase == KI_TRUE) {
                    
                }

                if (queryRes == KI_FALSE) {
                    /*
                     * The property is not present or is set to *null*. This is not an error. It just means we have no
                     * base profile and are, thus, done loading profiles.
                     */
                    hasBaseProfile = KI_FALSE;
                }
                KiPopPathComponent(profPath, KiPlatform_GetPathSeparator());

                errCode = KiPushPathComponent(profPath, KiPlatform_GetPathSeparator(), baseProfQuery[0].mp_strValue);
                if (errCode != KiErr_Ok) {
                    KiInternal_UnloadAllProfiles(profArr);
                    KiDestroyArray(profArr);

                    KiDestroyString(profPath);
                    return errCode;
                }
            }
        } while (hasBaseProfile == KI_TRUE);
    }
    KiDestroyString(profPath);

    gl_ProfileManState.mp_profiles = profArr;
    return KiErr_Ok;
}

static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(ProfileManager)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    KiInternal_UnloadAllProfiles(gl_ProfileManState.mp_profiles);

    KiDestroyArray(gl_ProfileManState.mp_profiles);
    return KiErr_Ok;
}
/** \endcond */


/** \cond */
KI_KRNLMOD_DEFINE(ProfileManager) {
    .m_structSize = sizeof(KiSModuleInfo),
    .mp_modUuid   = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId     = &KI_MAKE_STRING_VIEW("profile manager"),
    .m_modFlags   = 0,

    .mp_fnInit    = &KI_KRNLMOD_INITFN(ProfileManager),
    .mp_fnUninit  = &KI_KRNLMOD_UNINITFN(ProfileManager)
};
/** \endcond */


