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
 * \file  util.h
 * \brief defines various auxiliary data-structures and macros
 */


#pragma once

/* Kira includes */
#include <kira/def.h>


/** \cond INTERNAL */
/**
 */
#define __KI_CONCAT2_IMPL__(a, b)      a##b
/**
 */
#define __KI_CONCAT3_IMPL__(a, b, c)   __KI_CONCAT2_IMPL__(__KI_CONCAT2_IMPL__(a, b), c)
/**
 */
#define __KI_LINE_IMPL__               __LINE__
/**
 */
#define __KI_POW2_OR_IMPL__(x, n)      ((x) | ((x) >> (n)))
/**
 */
#define __KI_NEXTPOWOF2_IMPL__(x)      (((x) <= 1ULL) ? 1ULL : \
    (__KI_POW2_OR_IMPL__(                                      \
        __KI_POW2_OR_IMPL__(                                   \
            __KI_POW2_OR_IMPL__(                               \
                __KI_POW2_OR_IMPL__(                           \
                    __KI_POW2_OR_IMPL__(                       \
                        __KI_POW2_OR_IMPL__(                   \
                            (x) - 1ULL,                        \
                        1),                                    \
                    2),                                        \
                4),                                            \
            8),                                                \
        16),                                                   \
    32) + 1ULL))
/**
 */
#define __KI_TYPEOF_IMPL__(x)     \
    (KiEVariantType)_Generic((x), \
        default: 0                \
    )
/** \endcond */

/**
 */
#define KI_EXPAND(...)                 __VA_ARGS__
/**
 */
#define KI_STRUCT(...)                 struct { __VA_ARGS__ }
/**
 */
#define KI_UNION(...)                  union { __VA_ARGS__ }
/**
 */
#define KI_DONTCARE(t)                 &(t){ 0 }
/**
 */
#define KI_UNREFERENCED_PARAMETER(p)   ((KiTVoid)(p))
/**
 */
#define KI_CONCAT2(a, b)               __KI_CONCAT2_IMPL__(a, b)
/**
 */
#define KI_CONCAT3(a, b, c)            __KI_CONCAT3_IMPL__(a, b, c)
/**
 */
#define KI_CONCAT4(a, b, c, d)         __KI_CONCAT2_IMPL__(__KI_CONCAT3_IMPL__(a, b, c), d)
/**
 */
#define KI_CONCAT5(a, b, c, d, e)      __KI_CONCAT2_IMPL__(__KI_CONCAT3_IMPL__(a, b, c), __KI_CONCAT2_IMPL__(d, e))

/**
 */
#define KI_SIZEOF(...)                 ((KiTSize)(sizeof(struct __VA_ARGS__)))
/**
 */
#define KI_ELEMSIZE(arr)               ((KiTSize)(sizeof *arr))
/**
 */
#define KI_COUNTOF(arr)                ((KiTSize)(sizeof arr / sizeof *arr))
/**
 */
#define KI_OFFSETOF(str, f)            ((KiTOffset)(((size_t)((char*)&((str*)0)->f - (char*)0))))

/**
 * \defgroup McPadding Macros controlling structure padding
 * \brief    defines macros that allow padding to be added to structures in order for them to satisfy size and alignment
 *           requirements
 */
/** @{ */
/**
 * \def   KI_FORCEPADDING(n)
 * \brief manually inserts a padding of \c n bytes in the place where this macro appears
 * \note  For this macro to work, never use this macro 
 */
#define KI_FORCEPADDING(n)             KiTByte KI_CONCAT5(__pad, _, __KI_LINE_IMPL__, _, n)[n]
/**
 */
#define KI_FORCESIZE(n, ...)                                                                                      \
    union {                                                                                                       \
        KiTByte __forcesize_##n[n];                                                                               \
        struct __VA_ARGS__;                                                                                       \
                                                                                                                  \
        static_assert(                                                                                            \
            sizeof(struct __VA_ARGS__) <= n,                                                                      \
            "Size requirement of " #n " bytes cannot be satisfied. Payload is larger than " #n " bytes in total." \
        );                                                                                                        \
    }
/**
 */
#define KI_FORCESIZEPOW2(...)                                                                                          \
    union {                                                                                                            \
        unsigned char __pad_forcesizepow2__[(KiTSize)(__KI_NEXTPOWOF2_IMPL__((KiTUint64)sizeof(struct __VA_ARGS__)))]; \
        struct __VA_ARGS__;                                                                                            \
    }
/** @} */

/**
 */
#define KI_MIN(x, y)                   ((typeof (x))((x) > (y) ? (y) : (x)))
/**
 */
#define KI_MAX(x, y)                   ((typeof (x))((x) < (y) ? (y) : (x)))
/**
 */
#define KI_CLAMP(val, min, max)        (KI_MIN(KI_MAX(val, min), max))

/**
 */
#define KI_PACK8x2(v1, v2)             ((KiTUint16)(((KiTUint16)(v1)) << 8 | ((KiTUint16)(v2))))
/**
 */
#define KI_PACK8x4(v1, v2, v3, v4)     (((KiTUint32)(KI_PACK8x2(v1, v2))) << 16 | ((KiTUint32)(KI_PACK8x2(v3, v4))))
/**
 */
#define KI_PACK8x8(v1, v2, v3, v4, v5, v6, v7, v8) \
    (((KiTUint64)(KI_PACK8x4(v1, v2, v3, v4))) << 32 | ((KiTUint32)(KI_PACK8x4(v5, v6, v7, v8))))
/**
 */
#define KI_PACK16x2(v1, v2)            ((KiTUint32)(((KiTUint32)(v1)) << 16 | ((KiTUint32)(v2))))
/**
 */
#define KI_PACK16x4(v1, v2, v3, v4)    (((KiTUint64)(KI_PACK16x2(v1, v2))) << 32 | ((KiTUint64)(KI_PACK16x2(v3, v4))))
/**
 */
#define KI_PACK32x2(v1, v2)            ((KiTUint64)(((KiTUint64)(v1)) << 32 | ((KiTUint64)(v2))))

/**
 */
#define KI_INRANGE_INCL(val, min, max) ((KiTBool)((val) >= (min) && (val) <= (max)))
/**
 */
#define KI_INRANGE_EXCL(val, min, max) ((KiTBool)((val) > (min) && (val) < (max)))

/**
 */
#define KI_ISPOWEROFTWO(x)             ((KiTBool)(((x) != 0) && (((x) & ((x) - 1)) == 0)))
/**
 */
#define KI_P2LOG2(x)                                     \
    ( (KiTInt32 const[]){                                \
        0,  1, 48,  2, 57, 49, 28,  3,                   \
       61, 58, 50, 42, 38, 29, 17,  4,                   \
       62, 55, 59, 36, 53, 51, 43, 22,                   \
       45, 39, 33, 30, 24, 18, 12,  5,                   \
       63, 47, 56, 27, 60, 41, 37, 16,                   \
       54, 35, 52, 21, 44, 32, 23, 11,                   \
       46, 26, 40, 15, 34, 20, 31, 10,                   \
       25, 14, 19,  9, 13,  8,  7,  6                    \
    }[(((KiTUint64)(x)) * 0x03f79d71b4cb0a89ULL) >> 58])

/**
 */
#define KI_VERIFY_LUT(l, s)                                                     \
    static_assert(                                                              \
        KI_COUNTOF(l) == (KiTSize)(s),                                          \
        "Kismatch between actual and required size of lookup table \"" #l "\"." \
    );


/**
 * \defgroup KiBoolConst Boolean Constants
 * \brief    defines common boolean constants
 */
/** @{ */
#define KI_FALSE ((KiTBool)0)
#define KI_TRUE  ((KiTBool)1)
/** @} */


/**
 * \struct  KiSStringView
 * \brief   represents a compile-time constant string or a view, that is, a finite range into a C-string
 * \warning Strings of this type are not guaranteed to be <tt>NUL</tt>-terminated. Only read at most
 *              <tt>(KiTIntptr)mp_strPtr + (KiTIntptr)m_sizeInBytes</tt>
 *          bytes.
 */
KI_NATIVE typedef struct KiSStringView {
    KiTChar const *mp_strPtr;     /**< pointer to the first character in the string */
    KiTSize        m_sizeInBytes; /**< size of the string, in bytes (does NOT include the <tt>NUL</tt>-terminator) */
} KiSStringView;

/**
 * \def   KI_MAKE_STRING_VIEW(s)
 * \brief generates a compile-time constant \c KiSStringView from the given input string
 * \param s escaped string literal
 */
#define KI_MAKE_STRING_VIEW(s) (KiSStringView const){ .mp_strPtr = (KiTChar *)s, .m_sizeInBytes = sizeof s - 1 }


/**
 */
KI_NATIVE typedef struct KiSArrayView {
    KiTVoid const *mp_arrPtr;
    KiTSize        m_elemSize;
    KiTSize        m_elemCount;
} KiSArrayView;

/**
 */
#define KI_MAKE_ARRAY_VIEW(off, c, ...)                                                                   \
    (KiSArrayView const){                                                                                 \
        .mp_arrPtr   = ((KiTChar *)(KI_EXPAND(__VA_ARGS__)) + off * KI_ELEMSIZE(KI_EXPAND(__VA_ARGS__))), \
        .m_elemSize  = KI_ELEMSIZE(KI_EXPAND(__VA_ARGS__)),                                               \
        .m_elemCount = c                                                                                  \
    }


/**
 */
KI_NATIVE typedef struct KiSStaticArray {
    KiTVoid const *mp_arrPtr;
    KiTSize        m_elemSize;
    KiTSize        m_elemCount;
} KiSStaticArray;

/**
 */
#define KI_MAKE_STATIC_ARRAY(...)                           \
    (KiSStaticArray const){                                 \
        .mp_arrPtr   = KI_EXPAND(__VA_ARGS__),              \
        .m_elemSize  = KI_ELEMSIZE(KI_EXPAND(__VA_ARGS__)), \
        .m_elemCount = KI_COUNTOF(KI_EXPAND(__VA_ARGS__))   \
    }


/**
 */
KI_NATIVE typedef struct KiSVersion {
    KiTUint16 m_verMajor;
    KiTUint16 m_verMinor;
    KiTUint16 m_verPatch;
    KiTUint16 m_verRevision;
} KiSVersion;

/**
 * \def   KI_MAKE_VERSION(ma, mi, pa, rev)
 * \brief generates a compile-time \c KiSVersion constant with the given values
 * \param ma major version
 * \param mi minor version
 * \param pa patch version
 * \param re revision/build number
 */
#define KI_MAKE_VERSION(ma, mi, pa, rev)    \
    (KiSVersion const){                     \
        .m_verMajor    = ((KiTUint16)(ma)), \
        .m_verKinor    = ((KiTUint16)(mi)), \
        .m_verPatch    = ((KiTUint16)(pa)), \
        .m_verRevision = ((KiTUint16)(rev)) \
    }
/**
 * \def   KI_MAKE_VERSION_STRING(ma, mi, pa, rev)
 * \brief generates a compile-time C-string constant with the given values
 *
 * The resulting string will be of the form <tt>major.minor.patch-revision</tt>; e.g., "2.1.5-rev23". This specific form
 * will from now on be referred to as the <em>canonical version format</em>.
 * 
 * \param ma major version
 * \param mi minor version
 * \param pa patch version
 * \param re revision/build number
 */
#define KI_MAKE_VERSION_STRING(ma, mi, pa, rev) #ma"."#mi"."#pa"-rev"#rev


/**
 * \struct KiSLegalInformation
 * \brief  represents the Kira metadata legal information structure
 *
 * Many entities in Kira can have metadata attached to them, namely \c Interfaces, \c Components, \c Modules,
 * <tt>Test Suites</tt> and <tt>Test Cases</tt>. All of them can contain information regarding the author and legal
 * constraints when using the entity. This structure models the default Kira legal information object.
 */
KI_NATIVE typedef struct KiSLegalInformation {
    KiTSize       m_structSize; /**< size of this structure, in bytes */
    KiSStringView m_author;     /**< author identification (name, etc.) */
    KiSStringView m_contact;    /**< author contact information */
    KiSStringView m_license;    /**< license string identifier */
    KiSStringView m_copyright;  /**< copyright string */
    KiSStringView m_comment;    /**< optional comment regarding legal use */
} KiSLegalInformation;


/**
 */
KI_NATIVE typedef enum KiENumericRangeType : KiTUint32 {
    KiRgTy_Invalid  = 0,
    
    KiRgTy_Signed   = 1,
    KiRgTy_Unsigned = 2,
    KiRgTy_Float    = 3,

    __KiRgTy_Count__
} KiENumericRangeType;

/**
 */
KI_NATIVE typedef struct KiSNumericRange {
    KiENumericRangeType m_type;

    KI_STRUCT(
        KI_UNION(
            KiTInt64  m_iVal;
            KiTUint64 m_uVal;
            KiTDouble m_fVal;
        ) m_min;

        KI_UNION(
            KiTInt64  m_iVal;
            KiTUint64 m_uVal;
            KiTDouble m_fVal;
        ) m_max;
    );
} KiSNumericRange;

/**
 */
#define KI_MAKE_NUMERIC_RANGE(min, max)                                                                          \
    _Generic((min),                                                                                              \
        KiTInt8:   (KiSNumericRange){ .m_type = KiRgTy_Signed,   .m_min.m_iVal = (min), .m_max.m_iVal = (max) }, \
        KiTInt16:  (KiSNumericRange){ .m_type = KiRgTy_Signed,   .m_min.m_iVal = (min), .m_max.m_iVal = (max) }, \
        KiTInt32:  (KiSNumericRange){ .m_type = KiRgTy_Signed,   .m_min.m_iVal = (min), .m_max.m_iVal = (max) }, \
        KiTInt64:  (KiSNumericRange){ .m_type = KiRgTy_Signed,   .m_min.m_iVal = (min), .m_max.m_iVal = (max) }, \
        KiTUint8:  (KiSNumericRange){ .m_type = KiRgTy_Unsigned, .m_min.m_uVal = (min), .m_max.m_uVal = (max) }, \
        KiTUint16: (KiSNumericRange){ .m_type = KiRgTy_Unsigned, .m_min.m_uVal = (min), .m_max.m_uVal = (max) }, \
        KiTUint32: (KiSNumericRange){ .m_type = KiRgTy_Unsigned, .m_min.m_uVal = (min), .m_max.m_uVal = (max) }, \
        KiTUint64: (KiSNumericRange){ .m_type = KiRgTy_Unsigned, .m_min.m_uVal = (min), .m_max.m_uVal = (max) }, \
        KiTFloat:  (KiSNumericRange){ .m_type = KiRgTy_Float,    .m_min.m_fVal = (min), .m_max.m_fVal = (max) }, \
        KiTDouble: (KiSNumericRange){ .m_type = KiRgTy_Float,    .m_min.m_fVal = (min), .m_max.m_fVal = (max) }  \
    )


/**
 */
#define KI_VARIANT_GET(ty, var)             \
    _Generic((ty){ 0 },                     \
        KiTInt8:        (var).m_i8Val,      \
        KiTUint8:       (var).m_u8Val,      \
        KiTInt16:       (var).m_i16Val,     \
        KiTUint16:      (var).m_u16Val,     \
        KiTInt32:       (var).m_i32Val,     \
        KiTUint32:      (var).m_u32Val,     \
        KiTInt64:       (var).m_i64Val,     \
        KiTUint64:      (var).m_u64Val,     \
        KiTFloat:       (var).m_floatVal,   \
        KiTDouble:      (var).m_dblVal,     \
        KiTLongDouble:  (var).m_ldblVal,    \
        KiTChar *:      (var).mp_strVal,    \
        KiSStringView:  (var).m_svVal,      \
        KiTVoid *:      (var).mp_ptrVal,    \
        KiIBase *:      (var).mp_compVal,   \
        KiSVersion:     (var).m_verVal,     \
        KiSStaticArray: (var).m_stArrayVal, \
        KiSArrayView:   (var).m_avVal       \
    )
/**
 */
#define KI_VARIANT_SET(var, val) KiSetVariant(var, __KI_TYPEOF_IMPL__(val), val)

/** \cond */
KI_NATIVE typedef struct KiIBase                  KiIBase;
KI_NATIVE typedef enum   KiEErrorCode : KiTUint32 KiEErrorCode; 
/** \endcond */

/**
 */
KI_NATIVE typedef enum KiEVariantType {
    KiVarTy_Null = 0,

    KiVarTy_Other,
    KiVarTy_Boolean,
    KiVarTy_Int8,
    KiVarTy_Uint8,
    KiVarTy_Int16,
    KiVarTy_Uint16,
    KiVarTy_Int32,
    KiVarTy_Uint32,
    KiVarTy_Int64,
    KiVarTy_Uint64,
    KiVarTy_Float,
    KiVarTy_Double,
    KiVarTy_LongDouble,
    KiVarTy_String,
    KiVarTy_StringView,
    KiVarTy_Pointer,
    KiVarTy_Component,
    KiVarTy_Version,
    KiVarTy_StaticArray,
    KiVarTy_ArrayView,
    KiVarTy_NumericRange,

    __KiVarTy_Count__
} KiEVariantType;

/**
 */
KI_NATIVE typedef struct KiSVariant {
    KiEVariantType m_type;

    union {
        KiTInt8          m_i8Val;
        KiTUint8         m_u8Val;
        KiTInt16         m_i16Val;
        KiTUint16        m_u16Val;
        KiTInt32         m_i32Val;
        KiTUint32        m_u32Val;
        KiTInt64         m_i64Val;
        KiTUint64        m_u64Val;
        KiTFloat         m_floatVal;
        KiTDouble        m_dblVal;
        KiTLongDouble    m_ldblVal;
        KiTChar         *mp_strVal;
        KiSStringView    m_svVal;
        KiTVoid         *mp_ptrVal;
        KiIBase         *mp_compVal;
        KiSVersion       m_verVal;
        KiSStaticArray   m_stArrayVal;
        KiSArrayView     m_avVal;
        KiSNumericRange  m_rgVal;
    };
} KiSVariant;

/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiGetVariant(KiSVariant *varPtr, KiTVoid *resPtr);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiSetVariant(KiSVariant *varPtr, KiEVariantType varType, ...);


