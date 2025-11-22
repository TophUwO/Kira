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
 * \file  gparray.h
 * \brief defines the API for the kernel-level general-purpose array; can be used as a <em>slot vector</em> or a LIFO
 *        queue (stack)
 *
 * The general-purpose array can be used as a <em>LIFO queue</em> or a <em>slot vector</em>. A <em>slot vector</em> is a
 * data-structure that behaves like a normal vector (i.e., comparable to <tt>std::vector</tt>) except for the insertion
 * and erasure behavior. When a an element is inserted or erased, the subsequent elements are not shifted, leading to an
 * empty slot. If a new element is now inserted, it can be inserted into this empty slot, either by directly specifying
 * an index or by simply searching for the first free slot.
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE typedef struct KiSKrnlGPArray KiSKrnlGPArray;


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlGPArrayCreate(KiSKrnlGPArray **resPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlGPArrayDestroy(KiSKrnlGPArray *arrPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlGPArrayInsert(
    KiSKrnlGPArray *arrPtr,
    KiTVoid const *ePtr,
    KiTIndex *insIdx
);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiKrnlGPArrayErase(KiSKrnlGPArray *arrPtr, KiTIndex idx);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlGPArrayPush(KiSKrnlGPArray *arrPtr, KiTVoid const *ePtr);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiKrnlGPArrayPop(KiSKrnlGPArray *arrPtr);
/**
 */
KI_NATIVE extern KiTVoid **KI_CALL KiKrnlGPArrayMap(
    KiSKrnlGPArray const *arrPtr,
    KiTIndex offset,
    KiTSize count,
    KiTVoid ***beginPtr,
    KiTSize *nElem
);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiKrnlGPArrayAt(KiSKrnlGPArray const *arrPtr, KiTIndex slIndex);


