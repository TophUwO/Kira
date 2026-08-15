/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

/**
 * \file  app.h
 * \brief defines the public Kira runtime (i.e., client, application) API
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 * \typedef KiSReturnState
 * \struct  KiSReturnState
 */
KI_NATIVE typedef struct KiSReturnState {
    KiTSize      m_structSize;
    KiEErrorCode m_errCode;
    KiTBool      m_wantsRestart;
} KiSReturnState;


/**
 * \interface KiIEvent
 * \brief     represents a basic native \c Kira event
 *
 * \c KiIEvent serves as the base interface for all other event types. For ease of access and control, it is recommended
 * that, when you create a new event type, you compose the resulting interface of \c KiIEvent and the methods required
 * for your new event type. Every \c KiIEvent has a unique (numeric) event type associated with it which can be used for
 * fast processing/dispatch.<br>
 * Events are generally very short-lived objects only living until the control flow returns to the event queue. The
 * standard event types are mostly OS- and builtin Kira events. They are associated with one or more of the following
 * categories:
 * <ul>
 *  <li>high-level input events</li>
 *  <li>timer events</li>
 *  <li>window events</li>
 *  <li>application control events</li>
 * </ul>
 */
KI_INTERFACE(KiIEvent) KI_AUXILIARY {
    KI_METADATA(
        "uuid":    "70c4350a-43b9-4206-beed-b89980a0c2c2",
        "name":    "KiIEvent",
        "dname":   "IEvent",
        "sname":   "EV",
        "brief":   "represents a generic Kira application event",
        "version": [1, 0, 0],
        "legal":   {
            "author":    "Toffi",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2026 Toffi"
        }
    )
    KI_BASE(KiIEvent)

    /**
     */
    KiTUint64 (KI_CALL *GetEventType)(KiIEvent *self);

    /**
     */
    KiTVoid (KI_CALL *Accept)(KiIEvent *self);
    /**
     */
    KiTVoid (KI_CALL *Ignore)(KiIEvent *self);
};

/**
 * \interface KiIApplication
 * \brief     represents the \e client (= application), i.e., the central controlling instance of the software
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
            "author":    "Toffi",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2026 Toffi"
        }
    )
    KI_BASE(KiIApplication)

    /**
     * \fn    KiIApplication::Run(KiIApplication *self, KiSReturnState *resPtr)
     * \brief entrypoint of a Kira application
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
            "author":    "Toffi",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2026 Toffi"
        }
    )    
    KI_BASE(KiIEventDrivenApplication)

    /**
     */
    KiTVoid (KI_CALL *OnEvent)(KiIEventDrivenApplication *self, KiIEvent *evPtr);
};
