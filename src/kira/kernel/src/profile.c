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
 * \file  profile.c
 * \brief implements the kernel-level profile management system
 */


/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/profile.h>

#include <kira/kernel/int/string.h>
#include <kira/kernel/int/krnlmod.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef struct KiSProfileManagementState {
    KiSJson   *mp_profileJson;
    KiSString *mp_profilePath;
} KiSProfileManagementState;
/**
 */
static KiSProfileManagementState gl_ProfileManagement = { 0 };


/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(ProfileManagement)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /* Stub. */
    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(ProfileManagement)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /* Unload profile if necessary. */
    if (gl_ProfileManagement.mp_profileJson != nullptr)
        KiUnloadProfile();

    return KiErr_Ok;
}
/** \endcond */


KiEErrorCode KI_CALL KiLoadProfile(KiTChar const *profilePath) {
    KI_ASSERT(profilePath != nullptr, KiErr_InParameter);
    KI_ASSERT(*profilePath != '\0',   KiErr_InParameter);

    /* If a profile is currently loaded, we cannot load another one. */
    if (gl_ProfileManagement.mp_profileJson != nullptr)
        return KiErr_IllegalSystemState;

    /* Load the profile JSON. */
    gl_ProfileManagement.mp_profileJson = KiOpenJsonDocument(profilePath);
    if (gl_ProfileManagement.mp_profileJson == nullptr)
        return KiErr_LoadJsonDocument;

    /* Copy path. */
    KiEErrorCode errCode = KiCreateString(profilePath, &gl_ProfileManagement.mp_profilePath);
    if (errCode != KiErr_Ok) {
        /* Failed to copy path. How unfortunate. Need to unload the profile again. */
        KiCloseJsonDocument(gl_ProfileManagement.mp_profileJson);

        gl_ProfileManagement.mp_profileJson = nullptr;
        return errCode;
    }

    /* All good. */
    return KiErr_Ok;
}

KiTVoid KI_CALL KiUnloadProfile(KiTVoid) {
    /* We can unload when no profile is loaded. KiKrnlJsonClose() and KiKrnlStringDestroy() can handle nullptr. */
    KiCloseJsonDocument(gl_ProfileManagement.mp_profileJson);
    KiDestroyString(gl_ProfileManagement.mp_profilePath);

    /* Set the pointers back so that we can load another profile. */
    gl_ProfileManagement.mp_profileJson = nullptr;
    gl_ProfileManagement.mp_profilePath = nullptr;
}

KiSJson const *KI_CALL KiGetProfile(KiTVoid) {
    return (KiSJson const *)gl_ProfileManagement.mp_profileJson;
}

KiTChar const *KI_CALL KiGetCurrentProfilePath(KiTVoid) {
    return gl_ProfileManagement.mp_profilePath ? KiGetCString(gl_ProfileManagement.mp_profilePath) : "";
}


/** \cond */
KI_KRNLMOD_DEFINE(ProfileManagement) {
    .m_structSize = sizeof(KiSModuleInfo),
    .mp_modUuid   = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId     = &KI_MAKE_STRING_VIEW("profile management"),
    .m_modFlags   = 0,

    .mp_fnInit    = &KI_KRNLMOD_INITFN(ProfileManagement),
    .mp_fnUninit  = &KI_KRNLMOD_UNINITFN(ProfileManagement)
};
/** \endcond */


