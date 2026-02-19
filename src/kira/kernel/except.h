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
 * \file  except.h
 * \brief defines the private API for the Kira kernel exception system
 *
 * The Kira kernel exception system is a small component inside the kernel that allows the application to throw an
 * exception that is then handled by the kernel in a certain way. For example, this can be used to implement runtime
 * assertions.
 */


#pragma once

/* Kira includes */
#include <kira/dbg.h>


/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiSetExceptionHandler(KiEExceptionType excType, KiFExceptionHandler fnHnd);

/**
 */
KI_NATIVE KI_API KiTSize KI_CALL KiGetMaximumExceptionHandlerCount(KiTVoid);


