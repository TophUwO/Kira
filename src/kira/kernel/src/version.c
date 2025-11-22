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
 * \file  version.c
 * \brief implements some version comparison routines provided by the kernel
 */


/* Kira includes */
#include <kira/error.h>
#include <kira/util.h>

#include <kira/kernel/rt.h>

#include <kira/dbg/dbg.h>

#include <kira/test/test.h>


KiTInt8 KI_CALL KiKrnlCompareVersions(KiSVersion const *lVerPtr, KiSVersion const *rVerPtr) {
    KI_ASSERT(lVerPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(rVerPtr != nullptr, KiErr_InParameter);

    /* Of course, if the entities are literally the same, their version values are equal. */
    if (lVerPtr == rVerPtr)
        return 0;

    /* Pack them into one number that can be easily compared. */
    auto lvNum = KI_PACK16x4(lVerPtr->m_verMajor, lVerPtr->m_verKinor, lVerPtr->m_verPatch, lVerPtr->m_verRevision);
    auto rvNum = KI_PACK16x4(rVerPtr->m_verMajor, rVerPtr->m_verKinor, rVerPtr->m_verPatch, rVerPtr->m_verRevision);
    {
        /* Now, simply compare the numbers. */
        return (lvNum > rvNum) - (lvNum < rvNum);
    }

    /*
     * Because we intend to use 64-bit buffers to store version as mere numerals, we must assume that we can do so.
     * However, this should not actually ever be a problem unless KiSVersion is redefined to be larger or of greater
     * alignment than 64 bits.
     */
    static_assert(
        sizeof(KiSVersion) == sizeof(KiTUint64) && _Alignof(KiSVersion) <= _Alignof(KiTUint64),
        "\"KiSVersion\" must fit into one \"KiTUint64\" and have an alignment requirement lower than or equal to that "
        "of \"KiTUint64\"."
    );
}

KiTBool KI_CALL KiKrnlIsVersionInRange(
    KiSVersion const *verPtr,
    KiSVersion const *minVerPtr,
    KiSVersion const *maxVerPtr,
    KiTBool isInclusive
) {
    KI_ASSERT(verPtr != nullptr,    KiErr_InParameter);
    KI_ASSERT(minVerPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(maxVerPtr != nullptr, KiErr_InParameter);

    /* Compare against boundaries. */
    KiTInt8 const minCmpRes = KiKrnlCompareVersions(verPtr, minVerPtr);
    KiTInt8 const maxCmpRes = KiKrnlCompareVersions(verPtr, maxVerPtr);
    {
        /* Check if we are inside the bounds. */
        return isInclusive ? (minCmpRes >= 0 && maxCmpRes <= 0) : (minCmpRes > 0 && maxCmpRes < 0);
    }
}


/** \cond INTERNAL */
/**
 */
KI_COMPONENT(KiCVersionTester) KI_TEST {
    KI_TEST_METADATA(
        "uuid":      "d64268b4-8f30-4739-bb4f-c1713ce97d09",
        "version":   [1, 0, 0, 1],
        "brief":     "tests the version comparison routines",
        "legal":     {
            "author":    "TophUwO",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2025 TophUwO. All rights reserved."
        },
        "tags":      ["KERNEL", "SYSTEM", "VERSION"],
        "flags":     {
            "disabled": false
        }
    )
    KI_IMPLEMENTS(KiITestSuite) KI_PUBLIC;

    KiTRefCount m_refCount; /**< reference count */
};


/**
 */
KiTVoid KI_CALL KiCVersionTester_TestNullVersion(KiTVoid) KI_TEST {
    KI_TEST_METADATA(
        "uuid":    "eb1921c4-f258-4b9e-99a6-958fb07cc893",
        "version": [1, 0, 0, 1],
        "brief":   "tests whether invalid parameters are handled properly",
        "flags":   {
            "inheritTags": true
        }
    )

    KI_TEST_EXPECT_FAIL(KiKrnlCompareVersions(nullptr, &KI_MAKE_VERSION(28, 02, 2001, 1708)));
    KI_TEST_EXPECT_FAIL(KiKrnlCompareVersions(&KI_MAKE_VERSION(28, 02, 2001, 1708), nullptr));
    KI_TEST_EXPECT_FAIL(KiKrnlCompareVersions(nullptr, nullptr));
}

/**
 */
KiTVoid KI_CALL KiCVersionTester_TestEqualVersions(KiTVoid) KI_TEST {
    KI_TEST_METADATA(
        "uuid":      "1713d1a2-ce13-4959-8687-28f6eee38ef2",
        "version":   [1, 0, 0, 1],
        "brief":     "tests whether the routines can detect equal version numbers properly",
        "dependsOn": "TestNullVersion",
        "flags":     {
            "inheritTags": true
        }
    )

    /* Create two equal versions. */
    KiSVersion const lVer = KI_MAKE_VERSION(28, 02, 2001, 1708);
    KiSVersion const rVer = KI_MAKE_VERSION(28, 02, 2001, 1708);

    KI_TEST_ASSERT_EQ(KiKrnlCompareVersions(&lVer, &lVer), 0);
    KI_TEST_ASSERT_EQ(KiKrnlCompareVersions(&lVer, &rVer), 0);
}

/**
 */
KiTVoid KI_CALL KiCVersionTester_TestDistinctVersions(KiTVoid) KI_TEST {
    KI_TEST_METADATA(
        "uuid":      "ee469482-7e12-4b67-b2c8-f58d6e0f1e7d",
        "version":   [1, 0, 0, 1],
        "brief":     "tests whether distinct versions are detected as such",
        "dependsOn": "TestNullVersion",
        "flags":     {
            "inheritTags": true
        }
    )

    KiSVersion const v1 = KI_MAKE_VERSION(28, 02, 2001, 2025);
    KiSVersion const v2 = KI_MAKE_VERSION(28, 02, 2001, 1708);

    KI_TEST_ASSERT_EQ(KiKrnlCompareVersions(&v1, &v2),  1);
    KI_TEST_ASSERT_EQ(KiKrnlCompareVersions(&v2, &v1), -1);
}

/**
 */
KiTVoid KI_CALL KiCVersionTester_TestVersionRanges(KiTVoid) KI_TEST {
    KI_TEST_METADATA(
        "uuid":      "daaa71ac-4cec-47b1-b343-20cd5e821fae",
        "version":   [1, 0, 0, 1],
        "brief":     "tests whether version ranges are working",
        "dependsOn": "TestNullVersion",
        "flags":     {
            "inheritTags": true
        }
    )

    KiSVersion const v1 = KI_MAKE_VERSION(1, 0, 1, 123);
    KiSVersion const v2 = KI_MAKE_VERSION(1, 0, 1, 156);
    KiSVersion const v3 = KI_MAKE_VERSION(2, 2, 3, 1);
    KiSVersion const v4 = KI_MAKE_VERSION(7, 3, 1, 90);

    KI_TEST_ASSERT_TRUE(KiKrnlIsVersionInRange(&v3, &v3, &v4, KI_TRUE));
    KI_TEST_ASSERT_FALSE(KiKrnlIsVersionInRange(&v3, &v3, &v4, KI_FALSE));
    KI_TEST_ASSERT_TRUE(KiKrnlIsVersionInRange(&v2, &v1, &v2, KI_TRUE));
    KI_TEST_ASSERT_TRUE(KiKrnlIsVersionInRange(&v2, &v1, &v2, KI_FALSE));
    KI_TEST_ASSERT_FALSE(KiKrnlIsVersionInRange(&v1, &v3, &v3, KI_TRUE));
    KI_TEST_ASSERT_FALSE(KiKrnlIsVersionInRange(&v1, &v3, &v3, KI_FALSE));
    KI_TEST_ASSERT_FALSE(KiKrnlIsVersionInRange(&v4, &v1, &v1, KI_TRUE));
    KI_TEST_ASSERT_FALSE(KiKrnlIsVersionInRange(&v4, &v1, &v1, KI_FALSE));
    KI_TEST_ASSERT_TRUE(KiKrnlIsVersionInRange(&v4, &v4, &v4, KI_TRUE));
    KI_TEST_ASSERT_FALSE(KiKrnlIsVersionInRange(&v4, &v4, &v4, KI_FALSE));
}
/** \endcond */


