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
 * \file  array.h
 * \brief defines the API for the kernel-level general-purpose array; can be used as a <em>slot vector</em> or a LIFO
 *        queue (stack)
 *
 * The general-purpose array can be used as a <em>LIFO queue</em> (i.e., a <em>stack</em>) or a <em>slot vector</em>. A
 * <em>slot vector</em> is a data-structure that behaves like a normal vector (i.e., comparable to <tt>std::vector</tt>)
 * except for the insertion and erasure behavior. When a an element is inserted or erased, the subsequent elements are
 * not shifted, leading to an empty slot. If a new element is now inserted, it can be inserted into this empty slot,
 * either by directly specifying an index or by simply searching for the first free slot.
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE typedef struct KiSArray KiSArray;


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiCreateArray(KiSArray **resPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiDestroyArray(KiSArray *arrPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiClearArray(KiSArray *arrPtr);

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiInsertIntoArray(KiSArray *arrPtr, KiTVoid const *elemPtr, KiTIndex *insIdx);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiEraseFromArray(KiSArray *arrPtr, KiTIndex idx);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPushToArray(KiSArray *arrPtr, KiTVoid const *elemPtr);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiPopFromArray(KiSArray *arrPtr);

/**
 */
KI_NATIVE extern KiTVoid **KI_CALL KiMapArray(
    KiSArray const *arrPtr,
    KiTIndex offset,
    KiTSize count,
    KiTVoid ***beginPtr,
    KiTSize *nElem
);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiGetArrayElementAt(KiSArray const *arrPtr, KiTIndex slIndex);
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiGetArrayElementCount(KiSArray const *arrPtr);
