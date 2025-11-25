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
 * \file  cmdl.h
 * \brief defines the API for the Kira command-line argument parser
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 * \defgroup KiCmdlDSL Kira command-line argument language macros
 * \brief    contains macros used to specify a command-line schema
 */
/** @{ */
/**
 * \def   KI_COMMANDLINE(ident)
 * \brief opening directive of a command-line argument schema
 *
 * This directive opens the command-line argument schema. It generates a variable of type \c KiSCommandLineSchema. Since
 * the command-line argument parser may take parts of the schema and store it in its internal state for optimization
 * reasons, the lifetime of the schema must be global. This is normally achieved by prepending \c KI_COMMANDLINE(...)
 * with \c static and placing it in global scope or by placing the schema at the beginning of \c main(). This way, the
 * schema is in scope throughout the lifetime of the application.
 *
 * \param ident name of the variable representing the schema; must be unique to the current translation unit
 * 
 * \sa    \c KI_ARGUMENTS, \c KI_ARGUMENT, \c KI_SUBCOMMAND
 * \note  If \c KI_COMMANDLINE(...) is placed within a function, it has <em>automatic storage duration</em> (due to the
 *        DSL heavily utilizing inline pointers to compound literals being used to define the schema), meaning that you
 *        cannot use \c static with it. This has the side-effect that the schema is instantiated on the stack at
 *        run-time rather than placed in the \e data segment in case of <em>static storage duration</em> at compile-time.
 *        For this reason and because of the aforementioned lifetime concerns and -requirements, it is generally
 *        recommended to place the schema in global scope and use \c static to set its storage class.
 */
#define KI_COMMANDLINE(ident) KiSCommandLineSchema const ident = (KiSCommandLineSchema const)
/**
 * \def   KI_ARGUMENTS
 * \brief opens a sequence of (child-)arguments of the current (sub-)command
 * 
 * The Kira command-line argument parser supports 
 * 
 * \sa 
 */
#define KI_ARGUMENTS(...)     .mp_args = &KI_MAKE_STATIC_ARRAY((KiSCommandLineArgument const *const[])KI_EXPAND(__VA_ARGS__))
/**
 */
#define KI_SUBCOMMAND         &(KiSCommandLineArgument const)
/**
 */
#define KI_ARGUMENT           KI_SUBCOMMAND

/**
 */
#define KI_TYPE      .m_type =
/**
 */
#define KI_NAME      .mp_name =
/**
 */
#define KI_SPEC      .mp_spec =
/**
 */
#define KI_DESC      .mp_desc =
/**
 */
#define KI_PREFIX    .mp_prefixes =
/**
 */
#define KI_SEP       .mp_seps =
/**
 */
#define KI_DEFAULT   .mp_defVal =
/**
 */
#define KI_METAVAR   .mp_metaVar =
/**
 */
#define KI_FLAGS     .m_flags =
/**
 */
#define KI_HELPFMT   .m_helpFmt =
/**
 */
#define KI_DEFCHK    .m_defChk =
/**
 */
#define KI_ENUM(...) .mp_enum = &KI_MAKE_STATIC_ARRAY((KiSVariant const *const[])KI_EXPAND(__VA_ARGS__))
/**
 */
#define KI_BOUNDS    .mp_bounds = &KI_MAKE_NUMERIC_RANGE
/**
 */
#define KI_CHECK     .mp_checkCb =
/**
 */
#define KI_PROC      .mp_procCb =

/**
 */
#define KI_INT(x)        (&(KiSVariant const){ .m_type = KiVarTy_Int64, .m_i64Val = (x) })
/**
 */
#define KI_UINT(x)       (&(KiSVariant const){ .m_type = KiVarTy_Uint64, .m_u64Val = (x) })
/**
 */
#define KI_FLOAT(x)      (&(KiSVariant const){ .m_type = KiVarTy_Double, .m_dblVal = (x) })
/**
 */
#define KI_STRING(x)     (&(KiSVariant const){ .m_type = KiVarTy_String, .mp_strVal = (x) })
/**
 */
#define KI_BOOL(x)       (&(KiSVariant const){ .m_type = KiVarTy_Boolean, .m_u8Val = ((KiTBool)(x)) })
/**
 */
#define KI_RANGE(mi, ma) (&(KiSVariant const){ .m_type = KiVarTy_NumericRange, .m_rgVal = KI_MAKE_NUMERIC_RANGE(mi, ma) })
/** @} */


/** \cond */
KI_NATIVE typedef struct KiSCommandLineArgument  KiSCommandLineArgument;
KI_NATIVE typedef struct KiSCommandLineNamespace KiSCommandLineNamespace;
/** \endcond */

/**
 */
KI_NATIVE typedef KiEErrorCode (KI_CALL *KiFCommandLineArgumentCheck)(KiSCommandLineArgument const *aPtr, KiSVariant const *vPtr);
/**
 */
KI_NATIVE typedef KiTVoid (KI_CALL *KiFCommandLineArgumentProc)(KiSCommandLineArgument const *aPtr, KiSVariant *vPtr);


/**
 */
KI_NATIVE typedef enum KiECommandLineArgumentType {
    KiArgTy_Invalid,

    KiArgTy_String,
    KiArgTy_Bool,
    KiArgTy_Integer,
    KiArgTy_UnsignedInteger,
    KiArgTy_Float,
    KiArgTy_NumericRange,
    KiArgTy_SubCommand,

    KI_ENUM_COUNT(__KiArgTy_Count__)
} KiECommandLineArgumentType;

/**
 */
KI_NATIVE typedef enum KiECommandLineHelpFormat {
    KiHelpFmt_Option,
    KiHelpFmt_Command,

    KI_ENUM_COUNT(__KiHelpFmt_Count__)
} KiECommandLineHelpFormat;

/**
 */
KI_NATIVE typedef enum KiECommandLineArgumentFlags {
    KiArgFl_None            = 0,

    KiArgFl_Required        = (1 << 0),  // required
    KiArgFl_Optional        = (1 << 1),  // optional; also optional if required is not given and optional is not explicitly given
    KiArgFl_Deprecated      = (1 << 2),  // deprecated arg
    KiArgFl_NoHelp          = (1 << 3),  // do not autogen --help
    KiArgFl_Switch          = (1 << 4),  // store_true
    KiArgFl_InvSwitch       = (1 << 5),  // store_false
    KiArgFl_List            = (1 << 6),  // add to list
    KiArgFl_Countable       = (1 << 7),  // allow repeat or stack -v -v, or -vv to count it
    KiArgFl_BoundsBeginExcl = (1 << 8),  // when range is given as thing in m_span, make low excl
    KiArgFl_BoundsEndExcl   = (1 << 9),  // when range is given as thing in m_span, make high excl
    KiArgFl_Global          = (1 << 10), // only for options; option can appear anywhere; largest sub-tree is considered not just of the current subcmd
    KiArgFl_SubcmdsOptional = (1 << 11), // direct child subcommands are optional

    KI_ENUM_LAST(__KiArgFl_Last__, KiArgFl_SubcmdsOptional)
} KiECommandLineArgumentFlags;

/**
 */
KI_NATIVE typedef enum KiECommandLineSchemaFlags {
    KiSchFl_None            = 0,        /**< no special flags; default behavior */

    KiSchFl_NoHelp          = (1 << 0), /**< do not generate 'help' option/sub-command */
    KiSchFl_CaseInsensitive = (1 << 1),
    KiSchFl_Strict          = (1 << 2),
    KiSchFl_SubcmdsOptional = (1 << 3),

    KI_ENUM_LAST(__KiSchFl_Last__, KiSchFl_SubcmdsOptional)
} KiECommandLineSchemaFlags;

/**
 */
KI_NATIVE typedef enum KiECommandLineArgumentDefaultChecks {
    KiDefChk_None          = 0,

    KiDefChk_PathExists    = (1 << 0),
    KiDefChk_PathNotExists = (1 << 1),
    KiDefChk_ValueInRange  = (1 << 2),

    KI_ENUM_LAST(__KiDefChk_Last__, KiDefChk_ValueInRange)
} KiECommandLineArgumentDefaultChecks;


/**
 */
KI_NATIVE typedef struct KiSCommandLineArgument {
    KiTSize                             const        m_structSize;
    KiECommandLineArgumentType          const        m_type;
    KiTChar                             const *const mp_spec;
    KiTChar                             const *const mp_desc;
    KiTChar                             const *const mp_metaVar;
    KiSVariant                          const *const mp_defVal;
    KiSStaticArray                      const *const mp_enum;
    KiSNumericRange                     const *const mp_bounds;
    KiECommandLineArgumentFlags         const        m_flags;
    KiECommandLineHelpFormat            const        m_helpFmt;
    KiECommandLineArgumentDefaultChecks const        m_defChk;
    KiFCommandLineArgumentCheck         const        mp_checkCb;
    KiFCommandLineArgumentProc          const        mp_procCb;
    KiSStaticArray                      const *const mp_args;
} KiSCommandLineArgument;

/**
 */
KI_NATIVE typedef struct KiSCommandLineSchema {
    KiTSize                   const        m_structSize;
    KiTChar                   const *const mp_name;
    KiTChar                   const *const mp_desc;
    KiTChar                   const *const mp_prefixes;
    KiTChar                   const *const mp_seps;
    KiECommandLineSchemaFlags const        m_flags;
    KiECommandLineHelpFormat  const        m_helpFmt;
    KiSStaticArray            const *const mp_args;
} KiSCommandLineSchema;


/**
 */
KI_NATIVE KI_API KiSCommandLineNamespace *KI_CALL KiParseCommandLine(
    KiSCommandLineSchema const *cmdlSchemaPtr,
    int argc,
    char **argv
);
/**
 */
KI_NATIVE KI_API KiTVoid KI_CALL KiCleanupCommandLine(KiSCommandLineNamespace *nsPtr);

/**
 */
KI_NATIVE KI_API KiEErrorCode KI_CALL KiGetCommandLineError(KiSCommandLineNamespace const *nsPtr);
/**
 */
KI_NATIVE KI_API KiSCommandLineSchema const *KI_CALL KiGetCommandLineSchema(KiSCommandLineNamespace const *nsPtr);
/**
 */
KI_NATIVE KI_API KiSVariant KI_CALL KiGetCommandLineArgument(KiSCommandLineNamespace const *nsPtr, KiTChar const *aName);


/**
 * \page KiCmdlExamples Kira command-line parser
 *
 * Kira is a general-purpose component-based application development framework. This means it must be able to cater to 
 * various circumstances. One of the most basic of them being CLI (command-line interface) tools. Command-line parsing
 * is an issue for which many different solutions exist. Some of them are very simple and rigid while others are complex
 * but tedious to use. Kira tries to circumvent this problem by introducing a command-line parser that is both flexible
 * and easy to use. It solves this goal by --- instead of procedurally generating the command-line schema by the means
 * of <tt>AddArgument()</tt>- and <tt>AddSubParser()</tt>-like routines --- declaring the schema using a tree.
 * This page serves as the introduction to the parser and how to use it to express various CLI requirements: basic as
 * well as complex. On top of that, it will cover some pitfalls and implementation details.
 *
 * <h1>Basic Concepts</h1>
 * Before we get into the specifics on how get the most out of the command-line parser, we shall introduce some of its
 * core concepts --- namely the \e schema and the \e namespace. These are the main two significant entities the user has
 * to create and manage in order to interact with the parser.
 * <h2>Schema</h2>
 * The central configuration element of the command-line parser is the \e schema. The schema is essentially a tree-like
 * structure representing the nature of the CLI interface, specified using a custom DSL (the <em>Kira command-line
 * schema language</em> (KiSL)).
 * <h2>Namespace</h2>
 * The \e namespace is the second fundamental entity in the command-line parser. It contains the parsed command-line
 * arguments associated with their values in a form easily readable by the application.
 *
 * \code{.c}
 * KiEErrorCode KI_CALL CheckDigestSize(KiSCommandLineArgument const *aPtr, KiSVariant const *vPtr) {
 *     // Define the array of possible values.
 *     KiSStaticArray const arr = KI_MAKE_STATIC_ARRAY((KiTInt64 []){ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 });
 *     if (vPtr->m_type != KiVarTy_Int64) {
 *         // Return 'KiErr_InvalidArgType' if the argument type is incorrect.
 *         return KiErr_InvalidArgType;
 *     }
 * 
 *     // Check if the parsed value (vPtr) is in this array.
 *     for (KiTSize i = 0; i < arr.m_elemCount; i++)
 *         if (vPtr->m_i64Val == ((KiTInt64 *)arr.mp_arrPtr)[i]) {
 *             // Return 'KiErr_Ok' if a match has been found.
 *             return KiErr_Ok;
 *         }
 *     
 *     // Return 'KiErr_InvalidArgValue' if the parsed value is not valid.
 *     return KiErr_InvalidArgValue;
 * }
 * 
 * KiTVoid KI_CALL ProcDigestSize(KiSCommandLineArgument const *aPtr, KiSVariant *vPtr) {
 *     // Function that can be used to do post-processing on the actual value (vPtr).
 *     // You are allowed to modify the value in vPtr; the changes you make to vPtr
 *     // will be reflected when the argument is queried via KiGetCommandLineArgument().
 * }
 *     
 * static KI_COMMANDLINE(gl_c_MultiFunction) {
 *     KI_NAME    "kitool",
 *     KI_DESC    "example command line schema implementing a single devtool for generating hashes and uuids",
 *     KI_PREFIX  "-/",
 *     KI_SEP     "=:",
 *     KI_HELPFMT KiHelpFmt_Option,
 * 
 *     KI_ARGUMENTS({
 *         // global option arguments
 *         KI_ARGUMENT { KI_TYPE KiArgTy_Integer, KI_SPEC "--verbose;v", KI_DESC "enable verbose mode",      KI_FLAGS KiArgFl_Countable },
 *         KI_ARGUMENT { KI_TYPE KiArgTy_Bool,    KI_SPEC "--copy;c",    KI_DESC "copy result to clipboard", KI_FLAGS KiArgFl_Switch    },
 *         
 *         // sub-commands
 *         KI_SUBCOMMAND {
 *             KI_SPEC "hash",
 *             KI_DESC "generate SHA-512 hash of any input string",
 *             
 *             // arguments specific to the 'hash' sub-command
 *             KI_ARGUMENTS({
 *                 KI_ARGUMENT { KI_TYPE KiArgTy_String, KI_SPEC "input", KI_DESC "the input string", KI_FLAGS KiArgFl_Required },
 *                 KI_ARGUMENT {
 *                     KI_TYPE    KiArgTy_Integer,
 *                     KI_SPEC    "--size;s",
 *                     KI_DESC    "size of digest to return",
 *                     KI_CHECK   CheckDigestSize,
 *                     KI_PROC    ProcDigestSize,
 *                     KI_DEFAULT KI_INT(512),
 *                     KI_METAVAR "SIZE"
 *                 }
 *             })
 *         },
 *         KI_SUBCOMMAND {
 *             KI_SPEC  "uuid",
 *             KI_DESC  "generate version-4 UUIDs",
 *             KI_FLAGS KiArgFl_NoHelp | KiArgFl_Deprecated,
 *             
 *             // no arguments
 *             KI_ARGUMENTS({})
 *         },
 *         KI_SUBCOMMAND {
 *             KI_SPEC "range",
 *             KI_DESC "checks a numeric range for validity",
 *             
 *             KI_ARGUMENTS({
 *                 KI_ARGUMENT {
 *                     KI_TYPE    KiArgTy_NumericRange,
 *                     KI_SPEC    "in",
 *                     KI_DESC    "input range",
 *                     KI_FLAGS   KiArgFl_Optional,
 *                     KI_DEFAULT KI_RANGE(0.0, 1.0)
 *                 },
 *                 KI_ARGUMENT {
 *                     KI_TYPE    KiArgTy_Float,
 *                     KI_SPEC    "x",
 *                     KI_DESC    "example float value",
 *                     KI_DEFAULT KI_FLOAT(0.0f),
 *                     KI_BOUNDS  (-2.f, 2.f)
 *                 },
 *                 KI_ARGUMENT {
 *                     KI_TYPE    KiArgTy_String,
 *                     KI_SPEC    "--output-fmt;o",
 *                     KI_DESC    "output string format",
 *                     KI_DEFAULT KI_STRING("blake3"),
 *                     KI_ENUM    ({ KI_STRING("sha1"), KI_STRING("sha256"), KI_STRING("sha384"), KI_STRING("blake3") })
 *                 }
 *             })
 *         }
 *     })
 * };
 * \endcode
 */


