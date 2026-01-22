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
 * \file  compat.h
 * \brief carries out some toolset requirement checks and implements a thin compatibility layer between C11, C17, and
 *        C23
 *
 * It is possible to compile and use Kira with any compiler fully compliant with C11 or later, though, C23 is highly
 * recommended because it's simply much more convenient to work with.
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


