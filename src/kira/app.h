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


/**
 * \interface KiIApplication
 * \brief     represents the \e client (= application), i.e., the central controlling instance of the software system
 */
KI_INTERFACE(KiIApplication) KI_EXTENDS(KiIBase) KI_BUILTIN {
    /**
     * \brief reimplements \c KiIBase::QueryInterface()
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIApplication *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     * \brief reimplements \c KiIBase::Acquire()
     */
    KiTInt32 (KI_CALL *Acquire)(KiIApplication *self);
    /**
     * \brief reimplements \c KiIBase::Release
     */
    KiTInt32 (KI_CALL *Release)(KiIApplication *self);

    /**
     */
    KiTVoid (KI_CALL *Run)(KiIApplication *self);
};


