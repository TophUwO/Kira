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
 * \file  except.h
 * \brief defines the public API for the Kira kernel exception system
 *
 * The Kira kernel exception system is a small component inside the kernel that allows the application to throw an
 * exception that is then handled by the kernel in a certain way. For example, this can be used to implement runtime
 * assertions.
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/error.h>


/** \cond */
KI_NATIVE typedef struct KiSException KiSException;
/** \endcond */


/**
 */
KI_NATIVE typedef KiTVoid (KI_CALL *KiFExceptionHandler)(KiSException const *excPtr, KiTVoid *extraParam);


/**
 */
KI_NATIVE typedef enum KiEExceptionType {
    KiExcTy_Invalid         = 0,

    KiExcTy_AssertionFailed = 1,

    __KiExcTy_Count__
} KiEExceptionType;


/**
 */
KI_NATIVE typedef struct KiSException {
    KiTSize           m_structSize;
    KiEExceptionType  m_excType;
    KiTVoid          *mp_excDataPtr;
    KiTSize           m_excDataSize;
} KiSException;


/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiSetExceptionHandler(KiEExceptionType excType, KiFExceptionHandler fnHnd);

/**
 */
KI_NATIVE KI_API KiTSize KI_CALL KiGetMaximumExceptionHandlerCount(KiTVoid);


