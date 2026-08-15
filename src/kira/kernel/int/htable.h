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
 * \file  htable.h
 * \brief defines the API for the kernel-level hashtable
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE typedef struct KiSHashtable KiSHashtable;

/**
 */
KI_NATIVE typedef KiTUint64 (KI_CALL *KiFHashtableHash)(KiTVoid const *keyPtr, KiTUint64 const hashSeed);
/**
 */
KI_NATIVE typedef KiTBool   (KI_CALL *KiFHashtableKeyCmp)(KiTVoid const *lKeyPtr, KiTVoid const *rKeyPtr);
/**
 */
KI_NATIVE typedef KiTBool   (KI_CALL *KiFHashtableErasePred)(KiTVoid *keyPtr, KiTVoid *valPtr, KiTVoid *extraParam);


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiCreateHashtable(
    KiTSize initCap,
    KiFHashtableHash fnHash,
    KiFHashtableKeyCmp fnKeyCmp,
    KiSHashtable **resPtr
);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiDestroyHashtable(KiSHashtable *htPtr);

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiInsertIntoHashtable(KiSHashtable *htPtr, KiTVoid const *keyPtr, KiTVoid const *valPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiEraseFromHashtable(
    KiSHashtable *htPtr,
    KiTVoid const *keyPtr,
    KiTVoid **dstKeyPtr,
    KiTVoid **dstValPtr
);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiEraseFromHashtableIf(KiSHashtable *htPtr, KiFHashtableErasePred fnPred, KiTVoid *extraParam);

/**
 */
KI_NATIVE extern KiTBool KI_CALL KiIsInHashtable(KiSHashtable *htPtr, KiTVoid const *kPtr, KiTInt64 *iPtr);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiGetFromHashtable(KiSHashtable *htPtr, KiTVoid const *keyPtr);


