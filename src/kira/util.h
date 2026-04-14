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
#define __KI_CONCAT2_IMPL__(a, b)        a##b
/**
 */
#define __KI_CONCAT3_IMPL__(a, b, c)     __KI_CONCAT2_IMPL__(__KI_CONCAT2_IMPL__(a, b), c)
/**
 */
#define __KI_LINE_IMPL__                 __LINE__
/**
 */
#define __KI_POW2_OR_IMPL__(x, n)        ((x) | ((x) >> (n)))
/**
 */
#define __KI_NEXTPOWOF2_IMPL__(x)        (((x) <= 1ULL) ? 1ULL : \
    (__KI_POW2_OR_IMPL__(                                        \
        __KI_POW2_OR_IMPL__(                                     \
            __KI_POW2_OR_IMPL__(                                 \
                __KI_POW2_OR_IMPL__(                             \
                    __KI_POW2_OR_IMPL__(                         \
                        __KI_POW2_OR_IMPL__(                     \
                            (x) - 1ULL,                          \
                        1),                                      \
                    2),                                          \
                4),                                              \
            8),                                                  \
        16),                                                     \
    32) + 1ULL))
/**
 */
#define __KI_MKARR_IMPL__(t, s, ...) ((t const[s]){ __VA_ARGS__ })
/**
 */
#define __KI_TYPEOF_IMPL__(e)                \
    _Generic((e),                            \
        KiTInt8:        KiVarTy_Int8,        \
        KiTUint8:       KiVarTy_Uint8,       \
        KiTInt16:       KiVarTy_Int16,       \
        KiTUint16:      KiVarTy_Uint16,      \
        KiTInt32:       KiVarTy_Int32,       \
        KiTUint32:      KiVarTy_Uint32,      \
        KiTInt64:       KiVarTy_Int64,       \
        KiTUint64:      KiVarTy_Uint64,      \
        KiTFloat:       KiVarTy_Float,       \
        KiTDouble:      KiVarTy_Double,      \
        KiTLongDouble:  KiVarTy_LongDouble,  \
        KiTChar *:      KiVarTy_RawString,   \
        KiSStringView:  KiVarTy_StringView,  \
        KiTVoid *:      KiVarTy_Pointer,     \
        KiIBase *:      KiVarTy_Component,   \
        KiSVersion:     KiVarTy_Version,     \
        KiSStaticArray: KiVarTy_StaticArray, \
        KiSArrayView:   KiVarTy_ArrayView,   \
        default:        KiVarTy_Invalid      \
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
 * \def   KI_NOOP
 * \brief defines an operation that is a statement but never has side-effects
 *
 * The main use of this is in a switch- or if-statement (especially <tt>default</tt>-branch which could cause compiler
 * warnings if you do not handle all possible cases) where you might have to add a statement to the block in order to
 * shut up the compiler.
 */
#define KI_NOOP                         ((KiTVoid)(0))
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
#define KI_STRINGIFY(...)              #__VA_ARGS__

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
 * 
 */
#define KI_IGNORE_RETURN_VALUE(x)      ((KiTVoid)(x))

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
        _Static_assert(                                                                                           \
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
#define KI_MIN(x, y)                   ((x) > (y) ? (y) : (x))
/**
 */
#define KI_MAX(x, y)                   ((x) < (y) ? (y) : (x))
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
#define KI_BITMASK(...)                                                                                                     \
    ((KiTUint64 const){                                                                                                     \
          __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[0]  <<  0 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[1]  <<  1 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[2]  <<  2 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[3]  <<  3 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[4]  <<  4 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[5]  <<  5 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[6]  <<  6 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[7]  <<  7 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[8]  <<  8 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[9]  <<  9 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[10] << 10 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[11] << 11 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[12] << 12 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[13] << 13 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[14] << 14 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[15] << 15 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[16] << 16 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[17] << 17 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[18] << 18 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[19] << 19 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[20] << 20 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[21] << 21 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[22] << 22 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[23] << 23 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[24] << 24 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[25] << 25 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[26] << 26 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[27] << 27 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[28] << 28 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[29] << 29 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[30] << 30 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[31] << 31 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[32] << 32 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[33] << 33 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[34] << 34 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[35] << 35 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[36] << 36 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[37] << 37 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[38] << 38 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[39] << 39 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[40] << 40 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[41] << 41 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[42] << 42 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[43] << 43 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[44] << 44 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[45] << 45 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[46] << 46 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[47] << 47 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[48] << 48 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[49] << 49 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[50] << 50 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[51] << 51 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[52] << 52 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[53] << 53 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[54] << 54 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[55] << 55 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[56] << 56 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[57] << 57 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[58] << 58 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[59] << 59 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[60] << 60 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[61] << 61 \
        | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[62] << 62 | __KI_MKARR_IMPL__(KiTUint64, 64, __VA_ARGS__)[63] << 63 \
    })
/**
 */
#define KI_ISPOWEROFTWO(x)             ((KiTBool)(((x) != 0) && (((x) & ((x) - 1)) == 0)))
/**
 * \brief taken from https://www.chessprogramming.org/De_Bruijn_Sequence_Generator
 */
#define KI_P2LOG2(x)                                   \
    ((KiTInt32 const[64]){                             \
        0,  1,  2, 53,  3,  7, 54, 27,                 \
        4, 38, 41,  8, 34, 55, 48, 28,                 \
       62,  5, 39, 46, 44, 42, 22,  9,                 \
       24, 35, 59, 56, 49, 18, 29, 11,                 \
       63, 52,  6, 26, 37, 40, 33, 47,                 \
       61, 45, 43, 21, 23, 58, 17, 10,                 \
       51, 25, 36, 32, 60, 20, 57, 16,                 \
       50, 31, 19, 15, 30, 14, 13, 12                  \
    }[((KiTUint64)(x)) * 0x022fdd63cc95386dULL >> 58])

/**
 */
#define KI_VERIFY_LUT(l, s)                                                     \
    _Static_assert(                                                             \
        KI_COUNTOF(l) == (KiTSize)(s),                                          \
        "Mismatch between actual and required size of lookup table \"" #l "\"." \
    )
/**
 */
#define KI_VERIFY_TYPE(t1, t2)                                                                                    \
    _Static_assert(sizeof(t1) == sizeof(t2),     "Size of types \"" #t1 "\" and \"" #t2 "\" must be equal.");     \
    _Static_assert(_Alignof(t1) == _Alignof(t2), "Alignment of types \"" #t1 "\" and \"" #t2 "\" must be equal.")

/**
 */
#define KI_ENUM_COUNT(pre) __##pre##_Count__


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
KI_NATIVE typedef struct KiSUuid {
    KI_UNION(
        KI_STRUCT(
            KiTUint32 m_fPart;
            KiTUint16 m_sPart;
            KiTUint16 m_tPart;
            KiTUint64 m_qPart;
        );

        KiTByte m_bytes[16];
    );
} KiSUuid;

/**
 */
#define KI_MAKE_UUID(f, s, t, q) \
    (KiSUuid const){             \
        .m_fPart = (KiTUint32)f, \
        .m_sPart = (KiTUint16)s, \
        .m_tPart = (KiTUint16)t, \
        .m_qPart = (KiTUint64)q  \
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
 * \param rev revision/build number
 */
#define KI_MAKE_VERSION(ma, mi, pa, rev)    \
    (KiSVersion const){                     \
        .m_verMajor    = ((KiTUint16)(ma)), \
        .m_verMinor    = ((KiTUint16)(mi)), \
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
 * \param rev revision/build number
 */
#define KI_MAKE_VERSION_STRING(ma, mi, pa, rev) #ma"."#mi"."#pa"-rev"#rev


/**
 */
KI_NATIVE typedef enum KiENumericRangeType {
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
#define KI_MAKE_NUMERIC_RANGE(min, max)                                                                                \
    _Generic((min),                                                                                                    \
        KiTInt8:   (KiSNumericRange const){ .m_type = KiRgTy_Signed,   .m_min.m_iVal = (min), .m_max.m_iVal = (max) }, \
        KiTInt16:  (KiSNumericRange const){ .m_type = KiRgTy_Signed,   .m_min.m_iVal = (min), .m_max.m_iVal = (max) }, \
        KiTInt32:  (KiSNumericRange const){ .m_type = KiRgTy_Signed,   .m_min.m_iVal = (min), .m_max.m_iVal = (max) }, \
        KiTInt64:  (KiSNumericRange const){ .m_type = KiRgTy_Signed,   .m_min.m_iVal = (min), .m_max.m_iVal = (max) }, \
        KiTUint8:  (KiSNumericRange const){ .m_type = KiRgTy_Unsigned, .m_min.m_uVal = (min), .m_max.m_uVal = (max) }, \
        KiTUint16: (KiSNumericRange const){ .m_type = KiRgTy_Unsigned, .m_min.m_uVal = (min), .m_max.m_uVal = (max) }, \
        KiTUint32: (KiSNumericRange const){ .m_type = KiRgTy_Unsigned, .m_min.m_uVal = (min), .m_max.m_uVal = (max) }, \
        KiTUint64: (KiSNumericRange const){ .m_type = KiRgTy_Unsigned, .m_min.m_uVal = (min), .m_max.m_uVal = (max) }, \
        KiTFloat:  (KiSNumericRange const){ .m_type = KiRgTy_Float,    .m_min.m_fVal = (min), .m_max.m_fVal = (max) }, \
        KiTDouble: (KiSNumericRange const){ .m_type = KiRgTy_Float,    .m_min.m_fVal = (min), .m_max.m_fVal = (max) }  \
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
KI_NATIVE typedef struct KiIBase KiIBase;
/** \endcond */

/**
 */
KI_NATIVE typedef enum KiEVariantType {
    KiVarTy_Null    = 0,
    KiVarTy_Invalid = 1,

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


/**
 */
KI_NATIVE typedef struct KiSFunctionHandle {
    KI_UNION(
        KiTVoid (KI_CALL *mp_fnPtr)(KiTVoid);
        KiTVoid *mp_rawPtr;
    );
} KiSFunctionHandle;


/**
 * 
 */
KI_NATIVE typedef enum KiEFileAccessMode {
    KiFAccMd_Unknown      = 0,
    KiFAccMd_None         = KiFAccMd_Unknown,

    KiFAccMd_Read         = 1 << 0,
    KiFAccMd_Write        = 1 << 1,
    KiFAccMd_Append       = 1 << 2,

    KiFAccMd_Update       = 1 << 8,
    
    KiFAccMd_Binary       = 1 << 16,
    KiFAccMd_Text         = 1 << 17,

    KiFAccMd_MustExist    = 1 << 24,
    KiFAccMd_MustNotExist = 1 << 25
} KiEFileAccessMode;

/**
 * 
 */
KI_PLATFORM typedef enum KiEFilePositionOrigin {
    KiFPOri_Unknown = 0,

    KiFPOri_Set,
    KiFPOri_Cur,
    KiFPOri_End,

    __KiFPOri_Count__
} KiEFilePositionOrigin;


