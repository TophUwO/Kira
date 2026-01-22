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
 * \file  app.h
 * \brief defines the public Kira runtime (i.e., client, application) API
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/** \cond */
KI_NATIVE typedef struct KiIEvent KiIEvent;
/** \endcond */


/**
 */
KI_NATIVE typedef struct KiSReturnState {
    KiTSize      m_structSize;
    KiEErrorCode m_errCode;
    KiTBool      m_wantsRestart;
} KiSReturnState;


/**
 * \interface KiIApplication
 * \brief     represents the \e client (= application), i.e., the central controlling instance of the software system
 */
KI_INTERFACE(KiIApplication) KI_EXCLUDES(KiIEventDrivenApplication) KI_BUILTIN {
    KI_METADATA(
        "uuid":    "2d2b734f-10a5-473c-aace-cc59a01ffafe",
        "name":    "KiIApplication",
        "dname":   "IApplication",
        "sname":   "APP",
        "brief":   "provides the entrypoint for a generic application; can be anything; e.g., console, GUI, or real-time",
        "version": [1, 0, 0],
        "legal":   {
            "author":    "TophUwO",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2025 TophUwO"
        }
    )
    KI_BASE(KiIApplication)

    /**
     */
    KiTVoid (KI_CALL *Run)(KiIApplication *self, KiSReturnState *resPtr);
};

/**
 * \interface KiIEventDrivenApplication
 */
KI_INTERFACE(KiIEventDrivenApplication) KI_EXCLUDES(KiIApplication) KI_BUILTIN {
    KI_METADATA(
        "uuid":    "2d2b734f-10a5-473c-aace-cc59a01ffafe",
        "name":    "KiIEventDrivenApplication",
        "dname":   "IEventDrivenApplication",
        "sname":   "EVDRVAPP",
        "brief":   "provides the entrypoint for an application that is event-driven; e.g., a GUI application",
        "version": [1, 0, 0],
        "legal":   {
            "author":    "TophUwO",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2025 TophUwO"
        }
    )    
    KI_BASE(KiIEventDrivenApplication)

    /**
     */
    KiTVoid (KI_CALL *OnEvent)(KiIEventDrivenApplication *self, KiIEvent *evPtr);
};


