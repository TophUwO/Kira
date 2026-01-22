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
 * \file  dirctrl.c
 * \brief implements the kernel-level routines for retrieving and manipulating key directories (root directory, current
 *        working dir, etc.)
 */


/* stdlib includes */
#include <string.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/dir.h>
#include <kira/kernel/reg.h>

#include <kira/kernel/int/string.h>
#include <kira/kernel/int/krnlmod.h>
#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef struct KiSSystemDirectoryState {
    KiSString *mp_defWd;
    KiSString *mp_currWd;
    KiSString *mp_appRootDir;
} KiSSystemDirectoryState;

/**
 */
static KiSSystemDirectoryState gl_SysDirState;


/**
 */
KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(SystemDirectoryControl)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /* Clear the context memory. */
    memset(&gl_SysDirState, 0, sizeof gl_SysDirState);

    /* Initialize the current working directory as well as the default. */
    KiTChar const *currCwd = KiPlatform_GetCurrentWorkingDirectory();
    {
        /* Get cwd. */
        KiEErrorCode errCode = KiCreateString(currCwd, &gl_SysDirState.mp_currWd);
        if (errCode != KiErr_Ok)
            return errCode;

        /* Copy it into the default wd. */
        if ((errCode = KiDuplicateString(gl_SysDirState.mp_currWd, &gl_SysDirState.mp_defWd)) != KiErr_Ok) {
            KiDestroyString(gl_SysDirState.mp_currWd);

            KiPlatform_FreeString((KiTChar *)currCwd);
            return errCode;
        }
    }
    KiPlatform_FreeString((KiTChar *)currCwd);

    /* Initialize the application root directory. */
    KiTChar const *appDir = KiPlatform_GetApplicationRootDirectory();
    {
        KiEErrorCode errCode = KiCreateString(appDir, &gl_SysDirState.mp_appRootDir);
        if (errCode != KiErr_Ok) {
            KiDestroyString(gl_SysDirState.mp_currWd);
            KiDestroyString(gl_SysDirState.mp_defWd);

            KiPlatform_FreeString((KiTChar *)appDir);
            return errCode;
        }
    }
    KiPlatform_FreeString((KiTChar *)appDir);

    /* All good. */
    return KiErr_Ok;
}

/**
 */
KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(SystemDirectoryControl)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /* Restore the default working directory. */
    KiSetWorkingDirectory(KiGetCString(gl_SysDirState.mp_defWd));

    /* Destroy all of them. */
    KiDestroyString(gl_SysDirState.mp_appRootDir);
    KiDestroyString(gl_SysDirState.mp_currWd);
    KiDestroyString(gl_SysDirState.mp_defWd);
    return KiErr_Ok;
}
/** \endcond */


KiTChar const *KI_CALL KiGetApplicationRootDirectory(KiTVoid) {
    return KiGetCString(gl_SysDirState.mp_appRootDir);
}

KiTChar const *KI_CALL KiGetWorkingDirectory(KiTVoid) {
    return KiGetCString(gl_SysDirState.mp_currWd);
}

KiEErrorCode KI_CALL KiSetWorkingDirectory(KiTChar const *newWorkingDir) {
    KI_ASSERT(newWorkingDir != nullptr, KiErr_InParameter);
    KI_ASSERT(*newWorkingDir != '\0',   KiErr_InParameter);

    /* Set the working directory. */
    KiEErrorCode errCode = KiPlatform_SetCurrentWorkingDirectory(newWorkingDir);
    if (errCode != KiErr_Ok)
        return errCode;

    /* Overwrite the cached string. */
    errCode = KiAssignToString(gl_SysDirState.mp_currWd, newWorkingDir);
    if (errCode != KiErr_Ok) {
        /*
         * Because we failed to overwrite the cache string, we must restore the working directory. When
         * KiKrnlStringAssign() fails, the old string will still be valid. We can use this to restore it.
         */
        KiPlatform_SetCurrentWorkingDirectory(KiGetCString(gl_SysDirState.mp_currWd));

        return errCode;
    }

    /* All good. */
    return KiErr_Ok;
}

KiTVoid KI_CALL KiNativeSeparatorsToKiraSeparators(KiTChar *pathStr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InOutParameter);

    KiPlatform_CanonicalizeSeparators(pathStr);
}

KiTVoid KI_CALL KiSplitPath(
    KiTChar const *fullPathStr,
    KiSStringView *dirViewPtr, 
    KiSStringView *dirNameViewPtr,
    KiSStringView *fileNameViewPtr,
    KiSStringView *fileExtViewPtr
) {
    KI_ASSERT(fullPathStr != nullptr,     KiErr_InParameter);
    KI_ASSERT(dirViewPtr != nullptr,      KiErr_OutParameter);
    KI_ASSERT(dirNameViewPtr != nullptr,  KiErr_OutParameter);
    KI_ASSERT(fileNameViewPtr != nullptr, KiErr_OutParameter);
    KI_ASSERT(fileExtViewPtr != nullptr,  KiErr_OutParameter);

    KiTSize const fullLen = strlen(fullPathStr);
    {
        /** \todo IMPLEMENT */
    }
}


/** \cond */
KI_KRNLMOD_DEFINE(SystemDirectoryControl) {
    .m_structSize = sizeof(KiSModuleInfo),
    .mp_modUuid   = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId     = &KI_MAKE_STRING_VIEW("system directory control"),
    .m_modFlags   = 0,

    .mp_fnInit    = &KI_KRNLMOD_INITFN(SystemDirectoryControl),
    .mp_fnUninit  = &KI_KRNLMOD_UNINITFN(SystemDirectoryControl)
};
/** \endcond */


