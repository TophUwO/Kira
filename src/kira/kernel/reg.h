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
 * \file  reg.h
 * \brief defines the public API for the module-, component- and error stringifier registries
 */


#pragma once

/* Kira includes */
#include <kira/def.h>
#include <kira/util.h>
#include <kira/kcm.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiRegisterModule(KiTChar const *mdName, KiIModule *mdRef);
/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiUnregisterModule(KiTChar const *mdName);
/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiRegisterComponent(
    KiTChar const *apiName,
    KiIModule *mdRef,
    KiSComponentMetadata const *metaCompPtr,
    KiIComponentFactory *compFacRef
);
/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiUnregisterComponent(KiTChar const *apiName);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiIsRegistered(KiTChar const *apiName);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiCanRegister(KiTChar const *apiName);
/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiQueryComponent(
    KiTChar const *apiId,
    KiEInstanceScope instScope,
    KiTChar const *subsysId,
    KiIBase **resPtr
);


