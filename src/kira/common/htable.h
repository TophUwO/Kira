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
 * \file  htable.h
 * \brief defines the API for the kernel-level hashtable
 */


#pragma once

/* Kira includes */
#include <kira/def.h>

#include <kira/kernel/error.h>


/**
 */
KI_NATIVE typedef struct KiSKrnlHashtable KiSKrnlHashtable;

/**
 */
KI_NATIVE typedef KiTUint64 (KI_CALL *KiFKrnlHashtableHashFn)(KiTVoid const *keyPtr, KiTUint64 const hashSeed);
/**
 */
KI_NATIVE typedef KiTBool   (KI_CALL *KiFKrnlHashtableKeyCmpFn)(KiTVoid const *lKeyPtr, KiTVoid const *rKeyPtr);
/**
 */
KI_NATIVE typedef KiTBool   (KI_CALL *KiFHashtableErasePred)(KiTVoid *keyPtr, KiTVoid *valPtr, KiTVoid *extraParam);


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlHashtableCreate(
    KiTSize initCap,
    KiFKrnlHashtableHashFn fnHash,
    KiFKrnlHashtableKeyCmpFn fnKeyCmp,
    KiSKrnlHashtable **resPtr
);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlHashtableDestroy(KiSKrnlHashtable *htPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiKrnlHashtableInsert(
    KiSKrnlHashtable *htPtr,
    KiTVoid const *keyPtr,
    KiTVoid const *valPtr
);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlHashtableErase(
    KiSKrnlHashtable *htPtr,
    KiTVoid const *keyPtr,
    KiTVoid **dstKeyPtr,
    KiTVoid **dstValPtr
);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiKrnlHashtableEraseIf(
    KiSKrnlHashtable *htPtr,
    KiFHashtableErasePred fnPred,
    KiTVoid *extraParam
);
/**
 */
KI_NATIVE extern KiTBool KI_CALL KiSKrnlHashtableContains(
    KiSKrnlHashtable *htPtr,
    KiTVoid const *kPtr,
    KiTInt64 *iPtr
);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiKrnlHashtableAt(KiSKrnlHashtable *htPtr, KiTVoid const *keyPtr);


