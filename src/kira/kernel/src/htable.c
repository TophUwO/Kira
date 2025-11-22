/*****************************************************************************************************************
 * Kira - cross-platform 2-D role-playing game (RPG) game engine for desktop and mobile, and console platforms *
 *                                                                                                               *
 * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                   *
 *                                                                                                               *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of  *
 * this project. If this file is not present, visit                                                              *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                               *
 *****************************************************************************************************************/

/** \cond INTERNAL */
/**
 * \file  htable.c
 * \brief implements the kernel-level hashtable
 */


/* stdlib includes */
#include <stdlib.h>

/* Kira includes */
#include <kira/kernel/reg.h>

#include <kira/dbg/dbg.h>

#include <kira/kernel/int/htable.h>


/**
 */
#define KI_KRNLHT_EMPTY   ((KiTVoid *)nullptr)
/**
 */
#define KI_KRNLHT_DELETED ((KiTVoid *)(-1))
/**
 */
#define KI_KRNLHT_DEFCAP  ((KiTSize)(32))
/**
 */
#define KI_KRNLHT_MAXLF   ((KiTFloat)(0.75f))


KI_NATIVE typedef struct KiSKrnlHashtable {
    KiTSize                  m_elemCount;
    KiTSize                  m_elemCap;
    KiTUint64                m_hashSeed;
    KiFKrnlHashtableHashFn   mp_fnHash;
    KiFKrnlHashtableKeyCmpFn mp_fnKeyCmp;

    struct {
        KiTVoid   const *mp_key;
        KiTUint64        m_keyHash;
        KiTUint64        m_wealth;
        KiTVoid   const *mp_value;
    } *mp_tupleArr;
} KiSKrnlHashtable;


/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlHashtableInsertWithoutResize(
    KiSKrnlHashtable *htState,
    KiTVoid const *kPtr,
    KiTVoid const *vPtr
) {
    if (htState == nullptr) return KiErr_InOutParameter;
    if (kPtr == nullptr)    return KiErr_InParameter;
    
    /* Insert element. */
    auto tmpVal = (typeof(*htState->mp_tupleArr)){
        .mp_key    = (KiTChar *)kPtr,
        .m_keyHash = (*htState->mp_fnHash)(kPtr, htState->m_hashSeed),
        .m_wealth  = 0,
        .mp_value  = vPtr
    };
    /* Search for a spot to insert the element in. */
    for (KiTSize i = tmpVal.m_keyHash % htState->m_elemCap, currWealth = 0, j = 0; j < htState->m_elemCap;) {
        auto *const it = &htState->mp_tupleArr[i];

        /* Found free slot. Insert and return. */
        if (it->mp_key == KI_KRNLHT_EMPTY || it->mp_key == KI_KRNLHT_DELETED) {
            *it = (typeof(*it)){
                .mp_key    = tmpVal.mp_key,
                .m_keyHash = tmpVal.m_keyHash,
                .m_wealth  = currWealth,
                .mp_value  = tmpVal.mp_value
            };
            
            /* All good. */
            return KiErr_Ok;
        }

        /*
         * The current slot is not free. We must therefore compare the key's "wealth". If the wealth of the element
         * that is to be inserted is smaller or equal to the wealth of the item in the current probe slot, continue.
         * Otherwise, swap and continue with the new element.
         */
        if (currWealth > it->m_wealth) {
            auto tmpVal2   = *it;
            auto oldWealth = it->m_wealth;

            /* Insert. */
            *it = (typeof(*it)) {
                .mp_key    = tmpVal.mp_key,
                .m_keyHash = tmpVal.m_keyHash,
                .m_wealth  = currWealth,
                .mp_value  = tmpVal.mp_value
            };

            /* Update and continue insertion process with the new 'it'. */
            currWealth = oldWealth + 1;
            tmpVal     = tmpVal2;
        } else ++currWealth;
    }

    /*
     * Could not be inserted because the container is full (i.e, could not find a free slot.) This should essentially
     * never happen as we only insert without resizing if we know that there is enough space available. If we still
     * reach this point, there might be an issue with designating empty/deleted slots.
     */
    KI_ASSERT(KI_FALSE, KiErr_ContainerFull);
    return KiErr_ContainerFull;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlHashtableResize(KiSKrnlHashtable *htState, KiTSize newCap) {
    if (htState == nullptr)
        return KiErr_OutptrParameter;
    if (newCap * KI_KRNLHT_MAXLF <= htState->m_elemCount)
        return KiErr_InParameter;

    /* Create a new hashtable with the new properties. */
    KiSKrnlHashtable *newHt;
    KiEErrorCode errCode = KiKrnlHashtableCreate(newCap, htState->mp_fnHash, htState->mp_fnKeyCmp, &newHt);
    if (errCode != KiErr_Ok)
        return errCode;

    /* Insert all elements from the old hashtable into the new hashtable. */
    for (KiTSize i = 0, j = 0; j < htState->m_elemCount; i++) {
        auto const *const it = &htState->mp_tupleArr[i];

        /* Kiip empty slots. */
        if (it->mp_key == KI_KRNLHT_EMPTY || it->mp_key == KI_KRNLHT_DELETED)
            continue;

        /*
         * Insert tuple into new hashtable. This should never fail because the hashtable is definitely large enough and
         * even below the maximum allowed load factor.
         */
        KiInternal_KrnlHashtableInsertWithoutResize(newHt, it->mp_key, it->mp_value);
        ++j;
    }

    /*
     * Delete old hashtable tuple array and overwrite the entire hashtable with the one we newly created. Then, free the
     * structure memory of the new hashtable. Doing this will preserve the tuple array we just passed to the input
     * hashtable.
     */
    free(htState->mp_tupleArr);
    *htState = *newHt;
    free(newHt);

    /* All good. */
    return KiErr_Ok;
}


/**
 */
KiEErrorCode KI_CALL KrnlHashtableCreate(
    KiTSize initCap,
    KiFKrnlHashtableHashFn fnHash,
    KiFKrnlHashtableKeyCmpFn fnKeyCmp,
    KiSKrnlHashtable **resPtr
) {
    /**
     */
    extern KiTUint64 KI_CALL KiVirtual_KrnlHtGetRandomSeed(KiTVoid);

    if (initCap == 0)      return KiErr_InParameter;
    if (fnHash == nullptr) return KiErr_CallbackParameter;
    if (resPtr == nullptr) return KiErr_InOutParameter;

    /* Allocate hashtable memory. */
    if ((*resPtr = calloc(1, sizeof **resPtr)) == nullptr)
        return KiErr_MemoryAllocation;

    /* Allocate initial array. */
    typeof((*resPtr)->mp_tupleArr) initArr = calloc(1, sizeof *initArr * initCap);
    if (initArr == nullptr) {
        free(*resPtr);

        return KiErr_MemoryAllocation;
    }
 
    /* Init structure. */
    **resPtr = (KiSKrnlHashtable){
        .m_elemCount = 0,
        .m_elemCap   = initCap,
        .m_hashSeed  = KiVirtual_KrnlHtGetRandomSeed(),
        .mp_fnHash   = fnHash,
        .mp_tupleArr = initArr
    };
    /* All good. */
    return KiErr_Ok;
}

/**
 */
KiTVoid KI_CALL KiKrnlHashtableDestroy(KiSKrnlHashtable *htPtr) {
    if (htPtr == nullptr)
        return;

    /* It should be empty by now. If it isn't, then something is veeeeeery wrong. */
    KI_ASSERT(htPtr->m_elemCount == 0, KiErr_IllegalObjectState);

    /* Free hashtable array and state itself. */
    free(htPtr->mp_tupleArr);
    free(htPtr);
}

/**
 */
KiEErrorCode KI_CALL KiKrnlHashtableInsert(KiSKrnlHashtable *htPtr, KiTVoid const *keyPtr, KiTVoid const *valPtr) {
    if (htPtr == nullptr)  return KiErr_InOutParameter;
    if (keyPtr == nullptr) return KiErr_InParameter;

    /* If key exists, cannot add again. */
    if (KiSKrnlHashtableContains(htPtr, keyPtr, KI_DONTCARE(KiTInt64)))
        return KiErr_ItemAlreadyExists;

    /* Resize if needed. */
    if ((KiTFloat)(htPtr->m_elemCount + 1) / (KiTFloat)htPtr->m_elemCap >= KI_KRNLHT_MAXLF) {
        KiEErrorCode errCode = KiInternal_KrnlHashtableResize(htPtr, htPtr->m_elemCap * 2);

        if (errCode != KiErr_Ok)
            return errCode;
    }
    /* Insert element. */
    return KiInternal_KrnlHashtableInsertWithoutResize(htPtr, keyPtr, valPtr);
}

/**
 */
KiTBool KI_CALL KiSKrnlHashtableContains(KiSKrnlHashtable *htPtr, KiTVoid const *kPtr, KiTInt64 *iPtr) {
    if (htPtr == nullptr || kPtr == nullptr || iPtr == nullptr)
        return KI_FALSE;

    /* Calculate hash. */
    KiTUint64 const keyHash = (*htPtr->mp_fnHash)(kPtr, htPtr->m_hashSeed);
    {
        /* Try to find element. */
        for (KiTSize i = keyHash % htPtr->m_elemCap, j = 0; j < htPtr->m_elemCap; j++) {
            auto const *const it = &htPtr->mp_tupleArr[i];

            /*
             * Check for empty slot or if the current occupant's "wealth" (i.e., the key's probe distance) is lower than
             * the current probe distance.
             */
            if (it->mp_key == KI_KRNLHT_EMPTY || j > it->m_wealth) {
                /*
                 * If the slot has never been occupied yet (i.e., KI_KRNLHT_EMPTY), then we know that the key cannot be
                 * in here because otherwise it would be in here at last.
                 * Furthermore, if we encounter an element of which the probe distance is smaller than our current probe
                 * distance, we can conclude the key is not present. This is because RHH ensures that during insertion,
                 * keys with larger probe distances always displace those with smaller probe distances. Therefore, if
                 * our key had been inserted, it would have appeared before any element with a smaller probe distance.
                 */
                goto lbl_NOTFOUND;
            } else if (it->m_keyHash == keyHash && (*htPtr->mp_fnKeyCmp)(kPtr, it->mp_key)) {
                /*
                 * If element was found, return. Because the hash function could cause collisions, we still need to
                 * compare it directly for actual equality.
                 */
                *iPtr = (KiTInt64)i;

                return KI_TRUE;
            }

            i = (i + 1) % htPtr->m_elemCap;
        }
    }

lbl_NOTFOUND:
    /* Element was not found. */
    *iPtr = -1;
    
    return KI_FALSE;
}

/**
 */
KiTVoid KI_CALL KiKrnlHashtableErase(
    KiSKrnlHashtable *htPtr,
    KiTVoid const *keyPtr,
    KiTVoid **dstKeyPtr,
    KiTVoid **dstValPtr
) {
    if (htPtr == nullptr || keyPtr == nullptr || dstKeyPtr == nullptr || dstValPtr == nullptr)
        return;

    /* Locate key. */
    KiTInt64 locIdx;
    if (!KiSKrnlHashtableContains(htPtr, keyPtr, &locIdx)) {
        *dstKeyPtr = *dstValPtr = nullptr;

        return;
    }

    /*
     * Found the tuple that is to be removed. Remove it. Place tombstone (i.e., KI_KRNLHT_DELETED.) The hashtable does
     * not take ownership of the key and the value, so it does not take care of freeing the resourced used by them.
     */
    auto *const it = &htPtr->mp_tupleArr[locIdx];
    {
        /* Decrease count. */
        --htPtr->m_elemCount;

        /* Save the current key and value. */
        *dstKeyPtr = (KiTVoid *)it->mp_key;
        *dstValPtr = (KiTVoid *)it->mp_value;

        /* Overwrite the slots. */
        *it = (typeof(*it)){
            .mp_key    = KI_KRNLHT_DELETED,
            .m_keyHash = 0,
            .m_wealth  = 0,
            .mp_value  = nullptr
        };
    }
}

KiTVoid KI_CALL KiKrnlHashtableEraseIf(KiSKrnlHashtable *htPtr, KiFHashtableErasePred fnPred, KiTVoid *extraParam) {
    KI_ASSERT(htPtr != nullptr,  KiErr_InOutParameter);
    KI_ASSERT(fnPred != nullptr, KiErr_CallbackParameter);

    KiTSize nDel = 0;
    for (KiTSize i = 0, j = 0; j < htPtr->m_elemCount; i++) {
        auto *const it = &htPtr->mp_tupleArr[i];
        if (it->mp_key == KI_KRNLHT_EMPTY || it->mp_key == KI_KRNLHT_DELETED)
            continue;

        /* Need to delete? */
        if ((*fnPred)((KiTVoid *)it->mp_key, (KiTVoid *)it->mp_value, extraParam)) {
            /*
             * If the callback returned KI_TRUE, it means that the key was deleted. In this case, all we have to do is
             * clear the slot and decrement the element count. In order not to interfere with the iteration procedure,
             * we subtract the total number of entries erased.
             */
            *it = (typeof(*it)){
                .mp_key    = KI_KRNLHT_DELETED,
                .m_keyHash = 0,
                .m_wealth  = 0,
                .mp_value  = nullptr
            };

            ++nDel;
        }
        ++j;
    }

    /* Subtract number of entries erased. */
    htPtr->m_elemCount -= nDel;
}

/**
 */
KiTVoid *KI_CALL KiKrnlHashtableAt(KiSKrnlHashtable *htPtr, KiTVoid const *keyPtr) {
    KI_ASSERT(htPtr != nullptr,  KiErr_InOutParameter);
    KI_ASSERT(keyPtr != nullptr, KiErr_InParameter);

    /* Find element. */
    KiTInt64 elemIdx;
    if (!KiSKrnlHashtableContains(htPtr, keyPtr, &elemIdx))
        return nullptr;

    /* Found element. Return value. */
    return (KiTVoid *)htPtr->mp_tupleArr[elemIdx].mp_value;
}
/** \endcond */


