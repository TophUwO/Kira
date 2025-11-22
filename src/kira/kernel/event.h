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
 * \file  event.h
 * \brief defines the interfaces for the standard event types
 *
 * The standard event types are mostly OS- and builtin Kira events. They are associated with one or more of the
 * following categories:
 * <ul>
 *  <li>high-level input events</li>
 *  <li>timer events</li>
 *  <li>window events</li>
 *  <li>application control events</li>
 * </ul>
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 * \interface KiIEvent
 * \brief     represents a basic native Kira event
 *
 * \c KiIEvent serves as the base interface for all other event types. For ease of access and control, it is recommended
 * that, when you create a new event type, you compose the resulting interface of \c KiIEvent and the methods required
 * for your new event type. Every \c KiIEvent has a unique (numeric) event type associated with it which can be used for
 * fast processing/dispatch.<br>
 * Events are generally very short-lived objects, generally only living until the control flow returns to the event
 * queue.
 */
KI_INTERFACE(KiIEvent) KI_AUXILIARY {
    /**
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIEvent *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     */
    KiTInt32 (KI_CALL *Acquire)(KiIEvent *self);
    /**
     */
    KiTInt32 (KI_CALL *Release)(KiIEvent *self);

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


