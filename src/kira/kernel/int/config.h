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
 * \file  config.h
 * \brief defines the kernel-level API for the framework's runtime configuration facilities
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>

#include <kira/kernel/int/string.h>


/**
 */
KI_NATIVE KiSString *KI_CALL KiGetRootProfilePath(KiTVoid);
