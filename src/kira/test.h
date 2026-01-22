/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2025 TophUwO <tophuwo01@gmail.com>                                                                  *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

/**
 * \file  test.h
 * \brief defines the public API for the Kira unit testing framework (KiTest)
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/** \cond */
/**
 */
#define __KI_TEST_VERIFY_IMPL__(expr, op, exp) \
    do {                                       \
        if (!((expr) op (exp))) {              \
            /*KiTestReportFailure();*/         \
                                               \
            return;                            \
        }                                      \
    } while (0)
/**
 */
#define __KI_TEST_CHECKFL_IMPL__(expr, fl) \
    do {                                   \
        if (((expr) & (~(fl)))) {          \
            /*KiTestReportFailure();*/     \
                                           \
            return;                        \
        }                                  \
    } while (0)
/** \endcond */


/**
 */
#define KI_TEST

/**
 */
#define KI_TEST_ASSERT_TRUE(expr)  __KI_TEST_VERIFY_IMPL__((expr), ==, (KI_TRUE))
/**
 */
#define KI_TEST_ASSERT_FALSE(expr) __KI_TEST_VERIFY_IMPL__((expr), ==, (KI_FALSE))


/**
 */
#define KI_TEST_ASSERT_OK(expr)
/**
 */
#define KI_TEST_ASSERT_THROWS(expr)
/**
 */
#define KI_TEST_ASSERT_FAILS(expr, errCode)
/**
 */
#define KI_TEST_ASSERT_ERRCODE(expr, errCode) __KI_TEST_VERIFY_IMPL__((expr), ==, (errCode))

/**
 */
#define KI_TEST_ASSERT_EQ(expr, exp)          __KI_TEST_VERIFY_IMPL__((expr), ==, (exp))
/**
 */
#define KI_TEST_ASSERT_STREQ(expr, exp)       __KI_TEST_VERIFY_IMPL__(KiTestCompareStrings((expr), (exp)), ==, (0))
/**
 */
#define KI_TEST_ASSERT_NE(expr, exp)          __KI_TEST_VERIFY_IMPL__((expr), !=, (exp))
/**
 */
#define KI_TEST_ASSERT_STRNE(expr, exp)       __KI_TEST_VERIFY_IMPL__(KiTestCompareStrings((expr), (exp)), !=, (0))
/**
 */
#define KI_TEST_ASSERT_GT(expr, exp)          __KI_TEST_VERIFY_IMPL__((expr), >, (exp))
/**
 */
#define KI_TEST_ASSERT_GE(expr, exp)          __KI_TEST_VERIFY_IMPL__((expr), >=, (exp))
/**
 */
#define KI_TEST_ASSERT_LT(expr, exp)          __KI_TEST_VERIFY_IMPL__((expr), <, (exp))
/**
 */
#define KI_TEST_ASSERT_LE(expr, exp)          __KI_TEST_VERIFY_IMPL__((expr), <=, (exp))

/**
 */
#define KI_TEST_ASSERT_FLAGSET(expr, fl)      __KI_TEST_CHECKFL_IMPL__((expr), (exp))
/**
 */
#define KI_TEST_ASSERT_FLAGCLEAR(expr, fl)    __KI_TEST_CHECKFL_IMPL__((expr), (~(exp)))


/** \cond */
KI_NATIVE typedef struct KiITestSuite      KiITestSuite;
KI_NATIVE typedef struct KiITestController KiITestController;
/** \endcond */

/**
 */
KI_NATIVE typedef KiTVoid (KI_CALL *KiFTestCase)(KiITestSuite *suiteRef, KiITestController *controllerRef);


/**
 */
KI_NATIVE typedef enum KiETestSuiteFlags {
    KiTestSuiteFl_None      = 0,

    KiTestSuiteFl_Disabled  = (1 << 0),
    KiTestSuiteFl_Sandboxed = (1 << 1)
} KiETestSuiteFlags;

/**
 */
KI_NATIVE typedef enum KiETestCaseFlags {
    KiTestCaseFl_None      = 0,

    KiTestCaseFl_Disabled  = (1 << 0),
    KiTestCaseFl_Sandboxed = (1 << 1),
    KiTestCaseFl_Timed     = (1 << 2)
} KiETestCaseFlags;

/**
 * \enum  KiETestCaseResult
 * \brief represents a test case result as a numeric value
 */
KI_NATIVE typedef enum KiETestCaseResult {
    KiTestCaseRes_Unknown  = 0, /**< unknown/invalid test result */

    KiTestCaseRes_Pass     = 1, /**< test case was passed */
    KiTestCaseRes_Fail     = 2, /**< test case failed (assertion, exc, etc.) */
    KiTestCaseRes_Ignored  = 3, /**< test case did not run because it was ignored due to filters, tags, etc. */
    KiTestCaseRes_Disabled = 4, /**< test case did not run because it is disabled */

    KI_ENUM_COUNT(KiTestCaseRes)
} KiETestCaseResult;


/**
 */
KI_NATIVE typedef struct KiSTestAssertInfo {
    KiTSize       m_structSize;
    KiTUint64     m_line;
    KiSStringView m_rawString;
    KiSStringView m_assertType;
    KiSStringView m_expr;
    KiSStringView m_exp;
} KiSTestAssertInfo;

/**
 * \struct KiSTestTestMetadata
 * \brief  represents the static metadata for a test case as well as default settings
 *
 * As for the test case metadata, it is provided by the test case/suite author in the form of an inline JSON document.
 * In order to specify test case metadata, use the \c KI_TEST_METADATA macro. Many of the attributes have default values
 * that do not need to be provided by the user. For more information regarding defaults, consult the documentation for
 * the KiTest unit-testing framework.
 *
 * \note   All strings inside this struct are encoded as UTF-8. They are compile-time generated, so you can assume they
 *         are encoded properly.
 */
KI_NATIVE typedef struct KiSTestCaseMetadata {
    KiTSize              m_structSize;    /**< size of this structure, in bytes */
    KiSUuid              m_uuid;          /**< UUID of the test case */
    KiSStringView        m_fileName;      /**< file name this test case resides in (relative to module root directory) */
    KiTUint64            m_fileLine;      /**< line the test case starts on */
    KiSStringView        m_functionName;  /**< full name of the function representing the test case */
    KiSStringView        m_name;          /**< \e canonical test case name (name provided by case metadata) */
    KiSVersion           m_version;       /**< version of the test case */
    KiSStringView        m_suiteName;     /**< \e canonical suite name (name provided by suite metadata) this test case is associated with */
    KiSStringView       *mp_platforms;    /**< identifiers for the platforms this test is to be run on */
    KiTSize              m_nPlatforms;    /**< number of platform identifiers in \c mp_platforms */
    KiSStringView        m_brief;         /**< brief description of the test case */
    KiSStringView        m_details;       /**< detailed description about the test case */
    KiSLegalInformation  m_legalInfo;     /**< structure containing legal information regarding this test case */
    KiSStringView       *mp_tags;         /**< tags associated with the test case */
    KiTSize              m_nTags;         /**< number of tags in \c mp_testTags */
    KiSStringView       *mp_dependencies; /**< canonical test case names this test case depends on */
    KiTSize              m_nDependencies; /**< number of dependencies in \c mp_testDependencies */
    KiSTestAssertInfo   *mp_assertInfos;  /**< information on all asserts in order of declaration */
    KiTSize              m_nAsserts;      /**< total number of assertions in this test case */
    KiETestCaseFlags     m_flags;         /**< (default) flags for this test case */
    KiTUint64            m_defTimeoutMs;  /**< (default) timeout for the test case in milliseconds (ms) */
    KiSStringView        m_extraMetadata; /**< extra metadata (corresponds to \c extra property) as serialized JSON object */
    KiSStringView        m_fullMetadata;  /**< full metadata as serialized JSON object */
} KiSTestCaseMetadata;

/**
 */
KI_NATIVE typedef struct KiSTestSuiteMetadata {
    KiTSize               m_structSize;
    KiSComponentMetadata *mp_compMetadata;
    KiSStringView         m_fileName;
    KiSStringView         m_name;
    KiSTestCaseMetadata  *mp_testCases;
    KiTSize               m_nTestCases;
    KiSStringView        *mp_platforms;
    KiTSize               m_nPlatforms;
    KiSStringView        *mp_tags;         /**< tags that can be used to sort and filter test suites */
    KiTSize               m_nTags;         /**< number of suite tags in \c mp_suiteTags */
    KiSStringView        *mp_dependencies; /**< dependencies (canonical suite names) that need to be run before this suite runs */
    KiTSize               m_nDependencies; /**< number of elements in \c mp_suiteDependencies */
    KiETestCaseFlags      m_flags;         /**< flags specific to this particular test suite */
} KiSTestSuiteMetadata;

/**
 * \struct KiSTestAssertContext
 * \brief  represents the context of an KiTest assertion
 */
KI_NATIVE typedef struct KiSTestAssertContext {
    KiTSize           m_structSize; /**< size of this structure, in bytes */
    KiTUint64         m_fileLine;   /**< line the current assertion resides in (\c __LINE__ is good for most cases) */
    KiSStringView     m_expr;       /**< expression that was tested */
    KiSStringView     m_opId;       /**< identifier of the operation */
    KiSStringView     m_expValue;   /**< expected value of the tested expression */
    KiETestCaseResult m_expResult;  /**< expected result for the current assertion */
} KiSTestAssertContext;


/**
 * \interface KiITestSuite
 * \brief     represents a test case group
 *
 * Test cases in KiTest can be grouped by their test subject and -context. These groups allow the user to provide a
 * specific test context that all tests are using. Context control functions can be implemented that make sure that the
 * context remains consistent across test cases. Each test case must be associated with exactly one test suite. When
 * KiTest starts up, all test suites present in the runtime are discovered and worked through in scheduling order. For
 * more information on KiTest and all its features and inner workings, consult the KiTest documentation page.
 */
KI_INTERFACE(KiITestSuite) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    /**
     * \brief reimplements \c KiIBase::QueryInterface()
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiITestSuite *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     * \brief reimplements \c KiIBase::Acquire() 
     */
    KiTInt32 (KI_CALL *Acquire)(KiITestSuite *self);
    /**
     * \brief reimplements \c KiIBase::Release()
     */
    KiTInt32 (KI_CALL *Release)(KiITestSuite *self);

    /**
     */
    KiTVoid (KI_CALL *BeforeEach)(KiITestSuite *self);
    /**
     */
    KiTVoid (KI_CALL *AfterEach)(KiITestSuite *self);
    /**
     */
    KiTVoid (KI_CALL *BeforeAll)(KiITestSuite *self);
    /**
     */
    KiTVoid (KI_CALL *AfterAll)(KiITestSuite *self);
};

/**
 */
KI_INTERFACE(KiITestController) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    /**
     * \brief reimplements \c KiIBase::QueryInterface()
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiITestController *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     * \brief reimplements \c KiIBase::Acquire() 
     */
    KiTInt32 (KI_CALL *Acquire)(KiITestController *self);
    /**
     * \brief reimplements \c KiIBase::Release()
     */
    KiTInt32 (KI_CALL *Release)(KiITestController *self);

    /**
     */
    KiTBool (KI_CALL *CompareStrings)(KiITestController *self, KiTChar const *lStrPtr, KiTChar const *rStrPtr);

    /**
     */
    KiTVoid (KI_CALL *Throw)(KiITestController *self, KiEErrorCode errCode);
    /**
     */
    KiTVoid (KI_CALL *Submit)(KiITestController *self, KiSTestAssertContext const *assCtxtPtr);
    /**
     */
    KiTVoid (KI_CALL *Report)(KiITestController *self, KiETestCaseResult testCaseResId);
};


