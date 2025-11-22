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
 * \file  evq.h
 * \brief defines the public API for the Kira event queue
 *
 * The Kira event queue is used by the OS to supply the application with system messages as well as by kernel
 * services to communicate with each other. It can also be used by the client to pass around any type of data in a
 * thread-safe way. It is recommended, if your application implements one, that a client-defined thread pool
 * (e.g., a job queue) uses this queue to transfer data.
 */


/* Kira includes */
#include <kira/kcm.h>


/** \cond */
KI_NATIVE typedef struct KiSEvent KiSEvent;
/** \endcond */


/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiPostThreadEvent(KiTThreadId thrId, KiSEvent *eventPtr);


