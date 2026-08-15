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
 * \file  version.c
 * \brief implements some version comparison routines provided by the kernel
 */


/* Kira includes */
#include <kira/util.h>
#include <kira/dbg.h>

#include <kira/kernel/rt.h>


/** \cond INTERNAL */
static KiTInt8 KI_CALL KiInternal_CompareVersions(KiSVersion const *lVerPtr, KiSVersion const *rVerPtr) {
    KI_ASSERT(lVerPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(rVerPtr != nullptr, KiErr_InParameter);

    /* Of course, if the objects are literally the same, their version values are equal. */
    if (lVerPtr == rVerPtr)
        return 0;

    KiTUint64 lvNum = KI_PACK16x4(lVerPtr->m_verMajor, lVerPtr->m_verMinor, lVerPtr->m_verPatch, lVerPtr->m_verRevision);
    KiTUint64 rvNum = KI_PACK16x4(rVerPtr->m_verMajor, rVerPtr->m_verMinor, rVerPtr->m_verPatch, rVerPtr->m_verRevision);
    {
        /* Only one of the expressions can be true so it's either 1 or -1 or 0 if both are false. */
        return (lvNum > rvNum) - (lvNum < rvNum);
    }

    /*
     * Because we intend to use 64-bit buffers to store version as mere numerals, we must assume that we can do so.
     * However, this should not actually ever be a problem unless KiSVersion is redefined to be larger or of greater
     * alignment than 64 bits.
     */
    _Static_assert(
        sizeof(KiSVersion) == sizeof(KiTUint64) && _Alignof(KiSVersion) <= _Alignof(KiTUint64),
        "\"KiSVersion\" must fit into one \"KiTUint64\" and have an alignment requirement lower than or equal to that "
        "of \"KiTUint64\"."
    );
}
/** \endcond */


KiTBool KI_CALL KiIsVersionInRange(
    KiSVersion const *verPtr,
    KiSVersion const *minVerPtr,
    KiSVersion const *maxVerPtr,
    KiTBool isInclusive
) {
    KI_ASSERT(verPtr != nullptr,    KiErr_InParameter);
    KI_ASSERT(minVerPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(maxVerPtr != nullptr, KiErr_InParameter);

    /* Compare against boundaries. */
    KiTInt8 const minCmpRes = KiInternal_CompareVersions(verPtr, minVerPtr);
    KiTInt8 const maxCmpRes = KiInternal_CompareVersions(verPtr, maxVerPtr);
    {
        /* Check if we are inside the bounds. */
        return isInclusive ? (minCmpRes >= 0 && maxCmpRes <= 0) : (minCmpRes > 0 && maxCmpRes < 0);
    }
}

KiTInt8 KI_CALL KiCompareVersions(KiSVersion const *lVerPtr, KiSVersion const *rVerPtr) {
    KI_ASSERT(lVerPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(rVerPtr != nullptr, KiErr_InParameter);

    return KiInternal_CompareVersions(lVerPtr, rVerPtr);
}


