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
 * \file  array.c
 * \brief implements the kernel-level general-purpose array
 */


/* stdlib includes */
#include <stdlib.h>

#include <string.h>
#include <memory.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/reg.h>

#include <kira/kernel/int/array.h>



/** \cond INTERNAL */
/**
 */
#define KI_KRNLGPARR_NONE   ((KiTIndex)(-1))
/**
 */
#define KI_KRNLGPARR_DEFCAP ((KiTSize)(32))


/**
 */
struct KiSArray {
    KiTSize     m_elemCnt;
    KiTSize     m_elemCap;
    KiTIndex    m_firstEmpty;

    KiTVoid   **mpp_elemArr;
};


static KiTIndex KI_CALL KiInternal_ArrayFindFirstEmpty(KiSArray const *arrPtr, KiTIndex stIdx) {
    KI_ASSERT(arrPtr != nullptr,                                KiErr_InParameter);
    KI_ASSERT(KI_INRANGE_INCL(stIdx, 0, arrPtr->m_elemCap - 1), KiErr_IndexError);

    /* Search for first free index after the start index. */
    for (KiTIndex i = stIdx + 1; i < arrPtr->m_elemCap; i++)
        if (arrPtr->mpp_elemArr[i] != nullptr)
            return i;

    /* Could not find free index. Should not happen. */
    return KI_KRNLGPARR_NONE;
}

static KiEErrorCode KI_CALL KiInternal_ArrayResize(KiSArray *arrPtr, KiTSize newSize) {
    newSize = newSize == 0 ? KI_KRNLGPARR_DEFCAP : newSize;

    /* No free slot. Reallocate and then use the first slot after the currently last one. */
    KiTVoid **newArr = malloc(sizeof *newArr * newSize);
    {
        if (newArr == nullptr)
            return KiErr_MemoryAllocation;

        memcpy_s(newArr, newSize * sizeof *newArr, arrPtr->mpp_elemArr, arrPtr->m_elemCap * sizeof *newArr);
        memset(&newArr[arrPtr->m_elemCap], 0, sizeof *newArr * (newSize - arrPtr->m_elemCap));
    }

    /* Now destroy the old one. */
    free(arrPtr->mpp_elemArr);
    /* Update the structure. */
    *arrPtr = (KiSArray){
        .m_elemCnt    = arrPtr->m_elemCnt,
        .m_elemCap    = newSize,
        .m_firstEmpty = arrPtr->m_elemCap,
        .mpp_elemArr  = newArr
    };

    /* All good. */
    return KiErr_Ok;
}
/** \endcond */


KiEErrorCode KI_CALL KiCreateArray(KiSArray **resPtr) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    /* Allocate memory for structure. */
    *resPtr = malloc(sizeof **resPtr);
    if (*resPtr == nullptr)
        return KiErr_MemoryAllocation;

    /* Init state. */
    **resPtr = (KiSArray const){
        .m_elemCnt    = 0,
        .m_elemCap    = 0,
        .m_firstEmpty = KI_KRNLGPARR_NONE,
        .mpp_elemArr  = nullptr
    };
    return KiErr_Ok;
}

KiTVoid KI_CALL KiDestroyArray(KiSArray *arrPtr) {
    if (arrPtr == nullptr)
        return;

    /* Check that no elements are left. If there are, then something is very much wrong. */
    KI_ASSERT(arrPtr->m_elemCnt == 0, KiErr_IllegalObjectState);

    /* Destroy the array and the structure itself. */
    free(arrPtr->mpp_elemArr);
    free(arrPtr);
}

KiTVoid KI_CALL KiClearArray(KiSArray *arrPtr) {
    KI_ASSERT(arrPtr != nullptr, KiErr_InOutParameter);

    arrPtr->m_elemCnt    = 0;
    arrPtr->m_firstEmpty = 0;

    memset(arrPtr->mpp_elemArr, 0, sizeof *arrPtr->mpp_elemArr * arrPtr->m_elemCap);
}


KiEErrorCode KI_CALL KiInsertIntoArray(
    KiSArray *arrPtr,
    KiTVoid const *elemPtr,
    KiTIndex *insIdx
) {
    KI_ASSERT(arrPtr != nullptr,  KiErr_InOutParameter);
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(insIdx != nullptr,  KiErr_OutParameter);

    /* Find first free index. Resize if no free indices are available. */
    if (arrPtr->m_firstEmpty == KI_KRNLGPARR_NONE) {
        KiEErrorCode errCode = KiInternal_ArrayResize(arrPtr, arrPtr->m_elemCap << 1);

        if (errCode != KiErr_Ok)
            return errCode;
    }
    *insIdx = arrPtr->m_firstEmpty;

    /* Insert element into first free slot. */
    arrPtr->mpp_elemArr[*insIdx]  = (KiTVoid *)elemPtr;
    arrPtr->m_elemCnt            += 1;

    /*
     * Now, if there is at least one more free slot, we search for it. Only consider the slots after the current one
     * because our slot was guaranteed by the implementation to be the leftmost one that was free.
     */
    arrPtr->m_firstEmpty = arrPtr->m_elemCnt < arrPtr->m_elemCap
        ? KiInternal_ArrayFindFirstEmpty(arrPtr, *insIdx + 1)
        : KI_KRNLGPARR_NONE
    ;
    /* All good. */
    return KiErr_Ok;
}

KiTVoid *KI_CALL KiEraseFromArray(KiSArray *arrPtr, KiTIndex idx) {
    KI_ASSERT(arrPtr != nullptr,                              KiErr_InOutParameter);
    KI_ASSERT(KI_INRANGE_INCL(idx, 0, arrPtr->m_elemCap - 1), KiErr_IndexError);

    /* Retrieve and erase slot. */
    KiTVoid **slotAddr = KiGetArrayElementAt(arrPtr, idx), *resPtr = *slotAddr;
    *slotAddr = nullptr;

    /*
     * Determine the first free slot. If the current 'first empty' comes after the index we just deleted, this means the
     * index we just cleared is the 'first empty' slot.
     */
    arrPtr->m_firstEmpty = arrPtr->m_firstEmpty > idx ? idx : arrPtr->m_firstEmpty;

    /* Return previous element. */
    --arrPtr->m_elemCnt;
    return resPtr;
}

KiEErrorCode KI_CALL KiPushToArray(KiSArray *arrPtr, KiTVoid const *elemPtr) {
    KI_ASSERT(arrPtr != nullptr,  KiErr_InOutParameter);
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    KiTIndex const idx2Insert = arrPtr->m_elemCap - 1;

    /* Resize if the last slot is occupied. */
    if (arrPtr->mpp_elemArr[arrPtr->m_elemCap - 1] != nullptr) {
        KiEErrorCode errCode = KiInternal_ArrayResize(arrPtr, arrPtr->m_elemCap << 1);

        if (errCode != KiErr_Ok)
            return errCode;
    }

    /* Insert element. */
    arrPtr->mpp_elemArr[idx2Insert]  = (KiTVoid *)elemPtr;
    arrPtr->m_firstEmpty             = arrPtr->m_firstEmpty ^ KI_KRNLGPARR_NONE ? arrPtr->m_firstEmpty : idx2Insert + 1;
    arrPtr->m_elemCnt               += 1;

    /* All good. */
    return KiErr_Ok;
}

KiTVoid *KI_CALL KiPopFromArray(KiSArray *arrPtr) {
    KI_ASSERT(arrPtr != nullptr, KiErr_InOutParameter);

    return arrPtr->m_elemCnt > 0 
        ? KiEraseFromArray(arrPtr, arrPtr->m_elemCap - 1)
        : nullptr
    ;
}


KiTVoid **KI_CALL KiMapArray(
    KiSArray const *arrPtr,
    KiTIndex offset,
    KiTSize count,
    KiTVoid ***beginPtr,
    KiTSize *nElem
) {
    KI_ASSERT(arrPtr != nullptr,                                 KiErr_InParameter);
    KI_ASSERT(KI_INRANGE_INCL(offset, 0, arrPtr->m_elemCnt - 1), KiErr_IndexError);
    KI_ASSERT(count <= arrPtr->m_elemCnt - offset,               KiErr_SizeParameter);
    KI_ASSERT(beginPtr != nullptr,                               KiErr_OutptrParameter);
    KI_ASSERT(nElem != nullptr,                                  KiErr_OutParameter);

    *nElem    = arrPtr->m_elemCnt - offset;
    *beginPtr = &arrPtr->mpp_elemArr[offset];

    return *beginPtr;
}

KiTVoid *KI_CALL KiGetArrayElementAt(KiSArray const *arrPtr, KiTIndex slIndex) {
    KI_ASSERT(arrPtr != nullptr,                                             KiErr_InParameter);
    KI_ASSERT(KI_INRANGE_INCL(slIndex, 0, KI_MAX(0, arrPtr->m_elemCap - 1)), KiErr_IndexError);

    if (arrPtr->mpp_elemArr == nullptr || slIndex >= arrPtr->m_elemCap)
        return nullptr;

    return arrPtr->mpp_elemArr[slIndex];
}

KiTSize KI_CALL KiGetArrayElementCount(KiSArray const *arrPtr) {
    KI_ASSERT(arrPtr != nullptr, KiErr_InParameter);

    return arrPtr->m_elemCnt;
}


