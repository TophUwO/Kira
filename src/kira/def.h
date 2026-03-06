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
 * \file  def.h
 * \brief exposes some global definitions, typedefs, and some macros for annotation purposes
 */


#pragma once


/** \cond */
/* Detect C standard version. */
#if (__STDC_VERSION__ >= 202311L)
    #define KI_STD_C23
#elif (__STDC_VERSION__ >= 201710L)
    #define KI_STD_C17
#elif (__STDC_VERSION__ >= 201112L)
    #define KI_STD_C11
#else
    /* We need to at least have C11 support for Kira to work. */
    #error Kira requires full C11 compiler support. Use, e.g., -std=c11, or change to a newer toolset. 
#endif

/* What if some poor soul still uses a compiler incompliant with C23? Shame on them, obviously. */
#if ((!defined __cplusplus) && (!defined KI_STD_C23))
    #define constexpr const
    #define nullptr   ((KiTVoid *)(0))

    #if (!defined bool)
        #define bool _Bool
    #endif
#endif

/* We need to also fix static assertions if not done by the platform itself. */
#if (defined KI_STD_C23)
    #define _Static_assert static_assert
#endif
/** \endcond */

/**
 * \defgroup KiAPIMacros API Macro Definitions
 * \brief    contains all the main macros used by the API
 */
/** @{ */
#if (defined __cplusplus)
    #define KI_NATIVE extern "C"
#else
    #define KI_NATIVE
#endif
#if ((defined _WIN32) || (defined __CYGWIN__))
    #if (defined KI_KERNEL_EXPORT)
        #define KI_API __declspec(dllexport)
    #else
        #define KI_API __declspec(dllimport)
    #endif
#else
    #if (defined KI_KERNEL_EXPORT)
        #define KI_API __attribute__((visibility("default")))
    #else
        #define KI_API
    #endif
#endif
#if (defined KI_HOST_MSVC)
    #define KI_CALL     __cdecl
    #define KI_NORETURN __declspec(noreturn)
#else
    #define KI_CALL     __attribute__((cdecl))
    #define KI_NORETURN __attribute__((noreturn))
#endif

/**
 * \def   KI_PLATFORM
 * \brief marks a function as having to be implemented per-platform and has no default implementation
 * 
 * \par Remarks
 *   While this symbol has no significance to the compiler, it is there for annotation purposes. It makes it obvious
 *   what functions a developer porting Kira to a new platform must implement in order for it to work. Each per-platform
 *   function has detailed behavioral documentation attached to it.
 */
#define KI_PLATFORM
/** @} */


/**
 * \defgroup KiTypedefs Main Type Definitions
 * \brief    defines some common integer types for use by the APIs
 */
/** @{ */
typedef char               KiTInt8, KiTBool, KiTChar;
typedef char unsigned      KiTUint8, KiTByte, KiTFlags8;
typedef short              KiTInt16;
typedef short unsigned     KiTUint16, KiTFlags16;
typedef int                KiTInt32;
typedef int unsigned       KiTUint32, KiTFlags32, KiTThreadId;
typedef long unsigned      KiTUlong;
typedef long long          KiTInt64;
typedef long long unsigned KiTUint64, KiTFlags64;
typedef void               KiTVoid, *KiTDynLibHandle;
typedef float              KiTFloat;
typedef double             KiTDouble;
typedef long double        KiTLongDouble;
#if (defined KI_TARGET_X64)
    typedef KiTInt64  KiTIntptr, KiTOffset, KiTIndex;
    typedef KiTUint64 KiTSize;
#else
    typedef KiTInt32  KiTIntptr, KiTOffset, KiTIndex;
    typedef KiTUint32 KiTSize;
#endif

_Static_assert(sizeof(KiTByte)     == 1, "Size of type \"KiTByte\" must be exactly one byte.");
_Static_assert(sizeof(KiTInt16)    == 2, "Size of type \"KiTInt16\" must be exactly two bytes.");
_Static_assert(sizeof(KiTInt32)    == 4, "Size of type \"KiTInt32\" must be exactly four bytes.");
_Static_assert(sizeof(KiTInt64)    == 8, "Size of type \"KiTInt64\" must be exactly eight bytes.");
_Static_assert(_Alignof(KiTByte)   == 1, "Alignment requirement of type \"KiTByte\" must be exactly one byte.");
_Static_assert(_Alignof(KiTInt16)  == 2, "Alignment requirement of type \"KiTInt16\" must be exactly two bytes.");
_Static_assert(_Alignof(KiTInt32)  == 4, "Alignment requirement of type \"KiTInt32\" must be exactly four bytes.");
_Static_assert(_Alignof(KiTInt64)  == 8, "Alignment requirement of type \"KiTInt64\" must be exactly eight bytes.");
/** @} */


/**
 * \defgroup KiAlignMc Alignment Macros
 * \brief    allows specifying special alignment requirements
 */
/** @{ */
#define KI_A1  _Alignas(_Alignof(KiTInt8))
#define KI_A2  _Alignas(_Alignof(KiTInt16))
#define KI_A4  _Alignas(_Alignof(KiTInt32))
#define KI_A8  _Alignas(_Alignof(KiTInt64))
#define KI_A16 _Alignas(16)
#define KI_A32 _Alignas(32)
#define KI_A64 _Alignas(64)
/** @} */


/**
 * \defgroup KiLimitsMc Numeric Limits Macros
 * \brief    limits regarding Kira's bitness-specific numeric types
 */
/** @{ */
#if (defined KI_TARGET_X64)
    #define KI_INTPTR_WIDTH ((KiTSize)(sizeof(KiTUint64)))
    #define KI_OFFSET_WIDTH ((KiTSize)(sizeof(KiTUint64)))
    #define KI_INDEX_WIDTH  ((KiTSize)(sizeof(KiTUint64)))
    #define KI_SIZE_WIDTH   ((KiTSize)(sizeof(KiTUint64)))

    #define KI_INTPTR_MIN   ((KiTIntptr)(-9223372036854775807LL - 1))
    #define KI_INTPTR_MAX   ((KiTIntptr)(9223372036854775807LL))
    #define KI_OFFSET_MIN   ((KiTOffset)(KI_INTPTR_MIN))
    #define KI_OFFSET_MAX   ((KiTOffset)(KI_INTPTR_MAX))
    #define KI_INDEX_MIN    ((KiTIndex)(KI_INTPTR_MIN))
    #define KI_INDEX_MAX    ((KiTIndex)(KI_INTPTR_MAX))
    #define KI_SIZE_MIN     ((KiTSize)(0ULL))
    #define KI_SIZE_MAX     ((KiTSize)(18446744073709551615ULL))
#else
    #define KI_INTPTR_WIDTH ((KiTSize)(sizeof(KiTUint32)))
    #define KI_OFFSET_WIDTH ((KiTSize)(sizeof(KiTUint32)))
    #define KI_INDEX_WIDTH  ((KiTSize)(sizeof(KiTUint32)))
    #define KI_SIZE_WIDTH   ((KiTSize)(sizeof(KiTUint32)))

    #define KI_INTPTR_MIN   ((KiTIntptr)(-2147483647 - 1))
    #define KI_INTPTR_MAX   ((KiTIntptr)(2147483647))
    #define KI_OFFSET_MIN   ((KiTOffset)(KI_INTPTR_MIN))
    #define KI_OFFSET_MAX   ((KiTOffset)(KI_INTPTR_MAX))
    #define KI_INDEX_MIN    ((KiTIndex)(KI_INTPTR_MIN))
    #define KI_INDEX_MAX    ((KiTIndex)(KI_INTPTR_MAX))
    #define KI_SIZE_MIN     ((KiTSize)(0U))
    #define KI_SIZE_MAX     ((KiTSize)(4294967295U))
#endif
/** @} */

 
