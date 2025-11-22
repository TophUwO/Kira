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
 * \file  json.h
 * \brief defines the API for the kernel-level JSON reader
 *
 * This library only allows reading JSON because the kernel never writes it. Writing can be done via installing a
 * corresponding module in userspace.
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE typedef struct KiSJson KiSJson;


/**
 */
KI_NATIVE typedef enum KiEJsonValueType : KiTUint32 {
    KiJsonValTy_Invalid = 0,
    KiJsonValTy_Null    = 1,
    KiJsonValTy_Number  = 2,
    KiJsonValTy_Boolean = 3,
    KiJsonValTy_String  = 4,
    KiJsonValTy_Array   = 5,
    KiJsonValTy_Object  = 6,

    __KiJsonValTy_Count__
} KiEJsonValueType;


/**
 */
KI_NATIVE typedef struct KiSJsonValueQuery {
    KiTChar          const *mp_pathStr;
    KiEJsonValueType        m_reqType;
    KiEErrorCode            m_errCode;

    union {
        KiTBool          m_boolValue;
        KiTDouble        m_dblValue;
        KiTChar   const *mp_strValue;
        KiSJson   const *mp_arrValue;
        KiSJson   const *mp_objValue;
    };
} KiSJsonValueQuery;


/**
 */
KI_NATIVE KI_API KiSJson *KI_CALL KiOpenJsonDocument(KiTChar const *filePath);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiCloseJsonDocument(KiSJson *docPtr);
/**
 */
KI_NATIVE KI_API KiSJson *KI_CALL KiGetJsonElement(KiSJson const *docPtr, KiTChar const *propPath);
/**
 */
KI_NATIVE KI_API KiSJson *KI_CALL KiGetPreviousJsonElement(KiSJson const *elemPtr);
/**
 */
KI_NATIVE KI_API KiSJson *KI_CALL KiGetNextJsonElement(KiSJson const *elemPtr);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiGetJsonElementValue(KiSJson const *elemPtr, KiSJsonValueQuery *queryPtr);
/**
 */
KI_NATIVE KI_API KiTBool KI_CALL KiGetJsonElementValues(
    KiSJson const *elemPtr,
    KiSJsonValueQuery *queriesArr,
    KiTSize nQueries
);
/**
 */
KI_NATIVE KI_API KiEJsonValueType KI_CALL KiGetJsonElementType(KiSJson const *elemPtr);


