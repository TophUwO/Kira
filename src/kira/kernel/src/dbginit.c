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
 * \file  dbginit.c
 * \brief implements the tiny component that initializes the Kira debugging module
 */


/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/rt.h>
#include <kira/kernel/dbg.h>

#include <kira/kernel/int/krnlmod.h>


/** \cond INTERNAL */
/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(DebugModuleControl)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

#if (defined KI_CONFIG_DEBUG)
        /**
         * \brief represents the default debug options in case no overriding debug options were passed by the launcher
         *        component
         */
        static KiSDebugOptions constexpr gl_c_DefDbgOptions = {
            .m_structSize       = sizeof gl_c_DefDbgOptions,
            .m_useDetRng        = KI_TRUE,
            .m_isAssertsEnabled = KI_TRUE
        };

        KiSRuntimeSpecification const *rtSpecs = KiGetRuntimeSpecification();
        {
            return KiStartDebugSession(rtSpecs->mp_dbgOpts != nullptr ? rtSpecs->mp_dbgOpts : &gl_c_DefDbgOptions);
        }
#else
    /* No operation. */
    return KiErr_Ok;
#endif
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(DebugModuleControl)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    KiStopDebugSession();
    return KiErr_Ok;
}
/** \endcond */


/** \cond */
KI_KRNLMOD(DebugModuleControl, {
    &KI_MAKE_UUID(0xFFFFFFFF, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC)
}, {
    .mp_modUuid  = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId    = &KI_MAKE_STRING_VIEW("debug module"),
    .m_modFlags  = 0,

    .mp_fnInit   = &KI_KRNLMOD_INITFN(DebugModuleControl),
    .mp_fnUninit = &KI_KRNLMOD_UNINITFN(DebugModuleControl)
});
/** \endcond */


