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
 * \file  env.c
 * \brief implements the standard Kira command-line parser
 */


 // rules:
    //  (a) if countable
    //   (1) always present in ns but if not specified then its 0
    //   (2) must be int or uint
    //  (b) if switch or invswitch, must be bool
    //  (c) default must match type, else UB
    //  (d) prefer static const, keep shit alive throughout app, so beginning of main or static const
    //  (e) optional by default, except for sub-commands
    //  (f) if has default and is given without value, default is filled in, else if it is not given, default is also filled in
    //  (g) if has no default, value is required, else error
    //  (h) spec cannot be empty or ';' only; needs either at least one long and/or one short
    //  (i) at most one short; any number of long (given rule (h) is not violated)
    //  (j) KiArgFl_NoHelp is only for sub-commands
    //  (k) no positionals and sub-commands on the same level
    //  (l) no duplicates in either long names or short names on the same level
    //  (m) use 'subcmd[.subcmd[.subcmd...]].ident' when there are dupes in the same tree (not needed for disjunct trees (like sub-commands))
    //  (n) only positional order matters, options can appear anywhere
    //  (o) allow '--' to separate options from positionals
    //  (p) metavar is first long name by default written in all CAPS
    //  (q) '-Fval' and '-F val' are always allowed, other delims with KI_SEP
    //  (r) positionals can be LIST so everything up until the next flag is considered its value
    //  (s) spec/name and desc and type are always required and cannot be empty
    //  (t) ranges are given as decimals x..y or x-y
    //  (u) flag grouping (e.g., -vsx for -v -s -x) is not allowed unless all flags in the group take no value (are countable, switch or invswitch) except for last one
    //  (v) globals must be unique in their name; cannot appear anywhere in any other sub-tree
    //  (x) globals must appear only at the root scope
    //  (y) if a global is given after a sub-command, then global scope is searched and matched (globals are highest class citizens)
    //  (z) if count is given and bounds are specified, an error will be thrown if the count is not in range
    //  (aa) count is only valid for option flags (take no value)
    //  (ab) if negatable is specified, it must be switch or invswitch
    //  (ac) default if avail must be in choices if avail
    //  (ad) assume subcmd if type is 0

    // is subcmd if mpp_args is not nullptr, else is arg
    // is option arg when spec starts with '--', else is it positional
    // positionals cannot start with prefix
    // short names must be ascii chars

    // think of mutual exclusivity between flags
     // (a) required and optional are mutually exclusive
     // (b) switch, invswitch, list, countable are
     // (c) simply and the range and then check for power of 2
     // boundexcl* flags valid only for int, uint, float types


/* stdlib includes */
#include <stdlib.h>

#include <ctype.h>

/* external includes */
#include <kira/kernel/ext/utf8proc/utf8proc.h>

/* Kira includes */
#include <kira/dbg.h>
#include <kira/env.h>

#include <kira/kernel/unic.h>

#include <kira/kernel/int/buffer.h>
#include <kira/kernel/int/htable.h>
#include <kira/kernel/int/string.h>


/** \cond INTERNAL */
#if (!defined KI_ENV_MAXALIASES)
    /**
     */
    #define KI_ENV_MAXALIASES ((KiTSize)(16))
#endif

/**
 */
#define KI_ENV_NEWSPEC                                                                       \
    (KiSCommandLineArgumentSpecification){                                                   \
        .m_argName     = -1,                                                                 \
        .ma_aliases    = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }, \
        .m_argFlagName = -1,                                                                 \
        .m_metavarName = -1                                                                  \
    }
/**
 */
#define KI_ENV_NEWTOK                       \
    (KiSCommandLineParsingToken){           \
        .m_type      = KiCmdlTokTy_Unknown, \
        .m_len       = 0,                   \
        .mp_startPtr = nullptr              \
    }


/** \cond */
KI_NATIVE typedef struct KiSCommandLineArgumentSpecification KiSCommandLineArgumentSpecification;
/** \endcond */


/**
 */
KI_NATIVE typedef KiEErrorCode (KI_CALL *KiFCommandLineValidationRoutine)(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgument const *argPtr
);
/**
 */
KI_NATIVE typedef KiEErrorCode (KI_CALL *KiFCommandLineSpecParsingRoutine)(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgumentSpecification *specPtr,
    KiTChar **inpStr
);


/**
 */
KI_NATIVE typedef enum KiECommandLineArgumentCategory {
    KiCmdlAC_Invalid    = 0,

    KiCmdlAC_Positional = 1 << 0,
    KiCmdlAC_Option     = 1 << 1,
    KiCmdlAC_Command    = 1 << 2,

    KiCmdlAC_All        = KiCmdlAC_Positional | KiCmdlAC_Option | KiCmdlAC_Command,
    KiCmdlAC_Args       = KiCmdlAC_Positional | KiCmdlAC_Option
} KiECommandLineArgumentCategory;

/**
 */
KI_NATIVE typedef enum KiECommandLineParsingTokenType {
    KiCmdlTokTy_Unknown = 0,

    KiCmdlTokTy_Value,
    KiCmdlTokTy_LongPrefix,
    KiCmdlTokTy_ShortPrefix,
    KiCmdlTokTy_Comma,
    KiCmdlTokTy_BeginAliases,
    KiCmdlTokTy_EndAliases,
    KiCmdlTokTy_BeginGroup,
    KiCmdlTokTy_EndGroup,
    KiCmdlTokTy_KVSeparator,
    KiCmdlTokTy_End,

    KI_ENUM_COUNT(KiCmdlTokTy)
} KiECommandLineParsingTokenType;

/**
 */
KI_NATIVE typedef enum KiECommandLineArgumentNameType {
    KiCmdlArgNT_Invalid = 0,

    KiCmdlArgNT_Main,
    KiCmdlArgNT_Alias,
    KiCmdlArgNT_Short,
    KiCmdlArgNT_Metavar,

    KI_ENUM_COUNT(KiCmlArgNT)
} KiECommandLineArgumentNameType;


/**
 */
KI_NATIVE typedef struct KiSCommandLineParsingToken {
    KiECommandLineParsingTokenType  m_type;
    KiTUint32                       m_len;
    KiTChar                        *mp_startPtr;
} KiSCommandLineParsingToken;

/**
 */
KI_NATIVE typedef struct KiSCommandLineParsingMessageParameters {
    KiTChar      *mp_inpStr;
    KiTOffset     m_offset;
    KiEErrorCode  m_errCode;
    va_list       mp_extraArgs;
} KiSCommandLineParsingMessageParameters;

/**
 */
KI_NATIVE typedef struct KiSCommandLineArgumentSpecification {
    KiTOffset m_argName;
    KiTOffset ma_aliases[KI_ENV_MAXALIASES];
    KiTOffset m_argFlagName;
    KiTOffset m_metavarName;
} KiSCommandLineArgumentSpecification;

/**
 */
KI_NATIVE typedef struct KiSCommandLineNamespace {
    KiSCommandLineSchema *mp_schema;
    KiSString            *mp_path;
    KiSBuffer            *mp_strs;
    KiSBuffer            *mp_specs;
    KiSBuffer            *mp_diags;
    KiSHashtable         *mp_args;
    KiTBool               m_isError;
} KiSCommandLineNamespace;


#pragma region Schema-Utils
/**
 */
static KiECommandLineArgumentCategory KI_CALL KiInternal_CmdlClassifyArgument(KiSCommandLineArgument const *argPtr) {
    if (argPtr == nullptr || argPtr->mp_spec == nullptr || *argPtr->mp_spec == '\0')
        return KiCmdlAC_Invalid;

    /* Options start with '-' in any case. */
    if (*argPtr->mp_spec == '-')
        return KiCmdlAC_Option;

    /*
     * Otherwise, if we have child arguments, we assume it's a (sub-)command or, if not, a positional argument.
     * Positionals cannot start with a '-' or any other character in KI_SEP (if present), but are accepted here. In such
     * a case, an error would be thrown when validating the schema.
     */
    return argPtr->mpp_args != nullptr ? KiCmdlAC_Command : KiCmdlAC_Positional;
}
#pragma endregion


#pragma region Argument-Spec-Parser
/**
 */
static KiTVoid KI_CALL KiInternal_CmdlSpecSkipWhitespace(KiTChar **inpStr) {
    KI_ASSERT(inpStr != nullptr, KiErr_InOutptrParameter);

    KiTUint32 cp;
    while ((cp = KiToUnicodeCodepoint(*inpStr, KI_DONTCARE(KiTBool))) && KiIsUnicodeWhitespace(cp) == KI_TRUE)
        *inpStr += KiGetUtf8CharacterSize(cp);
}

/**
 */
static KiTVoid KI_CALL KiInternal_CmdlSpecContinue(KiTChar **inpStr) {
    KI_ASSERT(inpStr != nullptr, KiErr_InOutptrParameter);

    *inpStr = KiNextUtf8Character(*inpStr);
}

/**
 * \brief implements the lexer for the 
 */
static KiSCommandLineParsingToken *KI_CALL KiInternal_CmdlSpecProduceToken(
    KiTChar **inpStr,
    KiSCommandLineParsingToken *resPtr
) {
    KI_ASSERT(inpStr != nullptr,  KiErr_InptrParameter);
    KI_ASSERT(*inpStr != nullptr, KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr,  KiErr_OutParameter);

    *resPtr = (KiSCommandLineParsingToken){ 0 };
    {
        KiInternal_CmdlSpecSkipWhitespace(inpStr);

        switch (**inpStr) {
            case '-':
                KiInternal_CmdlSpecContinue(inpStr);

                if (**inpStr == '-') {
                    KiInternal_CmdlSpecContinue(inpStr);

                    resPtr->m_type = KiCmdlTokTy_LongPrefix;
                } else resPtr->m_type = KiCmdlTokTy_ShortPrefix;

                break;
            case '(':  KiInternal_CmdlSpecContinue(inpStr); resPtr->m_type = KiCmdlTokTy_BeginAliases; break;
            case ')':  KiInternal_CmdlSpecContinue(inpStr); resPtr->m_type = KiCmdlTokTy_EndAliases;   break;
            case '[':  KiInternal_CmdlSpecContinue(inpStr); resPtr->m_type = KiCmdlTokTy_BeginGroup;   break;
            case ']':  KiInternal_CmdlSpecContinue(inpStr); resPtr->m_type = KiCmdlTokTy_EndGroup;     break;
            case ',':  KiInternal_CmdlSpecContinue(inpStr); resPtr->m_type = KiCmdlTokTy_Comma;        break;
            case ':': 
            case '=':  ++*inpStr; resPtr->m_type = KiCmdlTokTy_KVSeparator;  break;
            case '\0':
                resPtr->m_type = KiCmdlTokTy_End;
                
                break;
            default:
                resPtr->mp_startPtr = (KiTChar *)*inpStr++;

                while (isalnum(**inpStr++))
                    ++resPtr->m_len;

                resPtr->m_type = KiCmdlTokTy_Value;
        }
    }

    return resPtr;
}

/**
 */
static KiTBool KI_CALL KiInternal_CmdlSpecCanAddIdentifier(
    KiSCommandLineArgumentSpecification const *specPtr,
    KiECommandLineArgumentNameType type
) {
    KI_ASSERT(specPtr != nullptr,                                               KiErr_InParameter);
    KI_ASSERT(KI_INRANGE_EXCL(type, KiCmdlArgNT_Invalid, __KiCmlArgNT_Count__), KiErr_EnumParameter);

    /** \cond */
    static KiTOffset const gl_c_NameOffTable[] = {
        [KiCmdlArgNT_Invalid] = -1,
        [KiCmdlArgNT_Main]    = KI_OFFSETOF(KiSCommandLineArgumentSpecification, m_argName),
        [KiCmdlArgNT_Alias]   = -1,
        [KiCmdlArgNT_Short]   = KI_OFFSETOF(KiSCommandLineArgumentSpecification, m_argFlagName),
        [KiCmdlArgNT_Metavar] = KI_OFFSETOF(KiSCommandLineArgumentSpecification, m_metavarName)
    };
    /** \endcond */

    return type == KiCmdlArgNT_Alias
        ? specPtr->ma_aliases[KI_COUNTOF(specPtr->ma_aliases) - 1] == -1
        : *(KiTOffset *)((KiTByte *)specPtr + gl_c_NameOffTable[type]) == -1
    ;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlSpecAddIdentifier(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgumentSpecification *specPtr,
    KiECommandLineArgumentNameType type,
    KiSStringView const *idView
) {
    KI_ASSERT(nsPtr != nullptr,                                                 KiErr_InOutParameter);
    KI_ASSERT(specPtr != nullptr,                                               KiErr_InOutParameter);
    KI_ASSERT(KI_INRANGE_EXCL(type, KiCmdlArgNT_Invalid, __KiCmlArgNT_Count__), KiErr_EnumParameter);
    KI_ASSERT(idView != nullptr,                                                KiErr_InParameter);

    /* Can we even add the identifier? */
    if (!KiInternal_CmdlSpecCanAddIdentifier(specPtr, type))
        return KiErr_SpecIdentLimitExceeded;

    /* Save the identifier in the namespace string buffer. */
    KiEErrorCode errCode = KiErr_Ok;
    KiTOffset    currOff = KiGetBufferPosition(nsPtr->mp_strs);
    {
        /* Write the identifier. */
        if ((errCode = KiWriteBufferData(nsPtr->mp_strs, idView->mp_strPtr, idView->m_sizeInBytes)) != KiErr_Ok)
            return errCode;

        /* Write NUL-terminator. */
        if ((errCode = KiWriteBufferData(nsPtr->mp_strs, "", sizeof "")) != KiErr_Ok) {
            KiSeekBufferPosition(nsPtr->mp_strs, currOff);

            return errCode;
        }
    }

    /* Finally add the identifier. */
    switch (type) {
        case KiCmdlArgNT_Alias: {
            KiTOffset *targetOff = &specPtr->ma_aliases[0];
            while (*targetOff != -1)
                ++targetOff;

            *targetOff = currOff;
            break;
        }
        case KiCmdlArgNT_Main:    specPtr->m_argName     = currOff; break;
        case KiCmdlArgNT_Short:   specPtr->m_argFlagName = currOff; break;
        case KiCmdlArgNT_Metavar: specPtr->m_metavarName = currOff; break;
        default:
            return KiErr_SpecIdentLimitExceeded;
    }

    /* All good. */
    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlSpecParseShortArg(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgumentSpecification *specPtr,
    KiTChar **inpStr
) {
    KI_ASSERT(nsPtr != nullptr,   KiErr_InOutParameter);
    KI_ASSERT(specPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(inpStr != nullptr,  KiErr_InOutptrParameter);

    KiSCommandLineParsingToken identTok;
    KiInternal_CmdlSpecProduceToken(inpStr, &identTok);
    {
        /* Next token must be an identifier. */
        if (identTok.m_type != KiCmdlTokTy_Value) {
            /// TODO: unexpected token

            return KiErr_UnexpectedSpecToken;
        }
        /* Length of the identifier must be exactly one. */
        if (identTok.m_len != 1) {
            /// TODO: wrong identifier (need length 1)

            return KiErr_SpecTokenSizeError;
        }
    }

    /* Ok, got a valid identifier. */
    return KiInternal_CmdlSpecAddIdentifier(nsPtr, specPtr, KiCmdlArgNT_Short, &(KiSStringView const){
        .mp_strPtr     = identTok.mp_startPtr,
        .m_sizeInBytes = identTok.m_len
    });
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlSpecParseLongArg(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgumentSpecification *specPtr,
    KiTChar **inpStr
) {
    KI_ASSERT(nsPtr != nullptr,   KiErr_InOutParameter);
    KI_ASSERT(specPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(inpStr != nullptr,  KiErr_InOutptrParameter);

    KiSCommandLineParsingToken currTok;
    KiInternal_CmdlSpecProduceToken(inpStr, &currTok);
    {
        /* Must be an identifier. */
        if (currTok.m_type != KiCmdlTokTy_Value) {
            /// TODO: unexpected token

            return KiErr_UnexpectedSpecToken;
        }

        /* Add it. */
        KiInternal_CmdlSpecAddIdentifier(nsPtr, specPtr, KiCmdlArgNT_Main, &(KiSStringView const){
            .mp_strPtr     = currTok.mp_startPtr,
            .m_sizeInBytes = currTok.m_len
        });
    }

    /* Parse a potential list of (alias) identifiers. */
    if (KiInternal_CmdlSpecProduceToken(inpStr, &currTok)->m_type == KiCmdlTokTy_BeginAliases) {
        while (KiInternal_CmdlSpecProduceToken(inpStr, &currTok)->m_type == KiCmdlTokTy_Value) {
            /* Add the identifier as an alias. */
            KiInternal_CmdlSpecAddIdentifier(nsPtr, specPtr, KiCmdlArgNT_Alias, &(KiSStringView const){
                .mp_strPtr     = currTok.mp_startPtr,
                .m_sizeInBytes = currTok.m_len
            });

            /* Need ',' token. */
            if (KiInternal_CmdlSpecProduceToken(inpStr, &currTok)->m_type != KiCmdlTokTy_Comma) {
                /// TODO: unexpected token

                return KiErr_UnexpectedSpecToken;
            }
        }

        /* Lastly, we need a ')' token. */
        if (currTok.m_type != KiCmdlTokTy_EndAliases) {
            /// TODO: missing token

            return KiErr_MissingSpecToken;
        }
    }

    /* All good. */
    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlSpecParseGroup(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgumentSpecification *specPtr,
    KiTChar **inpStr
) {
    KI_ASSERT(nsPtr != nullptr,   KiErr_InOutParameter);
    KI_ASSERT(specPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(inpStr != nullptr,  KiErr_InOutptrParameter);

    /* Accept '['. */
    KiSCommandLineParsingToken currTok;
    KiInternal_CmdlSpecProduceToken(inpStr, &currTok);
    {
        KiEErrorCode errCode = KiErr_Ok;

        /* What do we have? A long ident or a flag? */
        switch (currTok.m_type) {
            case KiCmdlTokTy_LongPrefix:
                errCode = KiInternal_CmdlSpecParseLongArg(nsPtr, specPtr, inpStr);

                break;
            case KiCmdlTokTy_ShortPrefix:
                errCode = KiInternal_CmdlSpecParseShortArg(nsPtr, specPtr, inpStr);

                break;
            default:;
                /// TODO: unexpected token
        }

        /* Parsed first group. Do we have a comma? */
        if (KiInternal_CmdlSpecProduceToken(inpStr, &currTok)->m_type != KiCmdlTokTy_Comma) {

        }
    }
    /* Accept ']'. */
    if (currTok.m_type != KiCmdlTokTy_EndGroup) {
        /// TODO: unexpected token

        return KiErr_UnexpectedSpecToken;
    }

    /* All good. */
    return KiErr_Ok;
}
#pragma endregion


#pragma region Schema-Validation
/**
 * \brief incompatibility matrix for command-line argument flags
 *
 * Each entry in the incompatibility matrix contains a bitmask where every 1 bit at position p corresponds to a flag the
 * current (i.e., the flag 1 << r where r is the row the bitmask resides in the matrix) flag is incompatible with,
 * specifically, the flag for which log2(flag) == p is true.<br>
 * \code{.c}
 * // Bit at index [n] corresponds to flag 1 << [n].
 * // Bit [0] is always LSB.
 * KI_BITMASK([0] = 0, [1] = 1, [2] = 0, [3] = 1, [4] = 1, ...)
 * \endcode
 *
 * To aid in maintaining the matrix and the bitmasks, the following table lists the flags and their corresponding
 * row index in the matrix which is also their position in the bitmask (left-to-right).
 * <table>
 *  <tr><th>Index</th> <th>Flag</th></tr>
 *
 *  <tr><td> 0</td> <td>KiArgFl_Required       </td></tr>
 *  <tr><td> 1</td> <td>KiArgFl_Optional       </td></tr>
 *  <tr><td> 2</td> <td>KiArgFl_Deprecated     </td></tr>
 *  <tr><td> 3</td> <td>KiArgFl_NoHelp         </td></tr>
 *  <tr><td> 4</td> <td>KiArgFl_Switch         </td></tr>
 *  <tr><td> 5</td> <td>KiArgFl_InvSwitch      </td></tr>
 *  <tr><td> 6</td> <td>KiArgFl_List           </td></tr>
 *  <tr><td> 7</td> <td>KiArgFl_Countable      </td></tr>
 *  <tr><td> 8</td> <td>KiArgFl_BoundsBeginExcl</td></tr>
 *  <tr><td> 9</td> <td>KiArgFl_BoundsEndExcl  </td></tr>
 *  <tr><td>10</td> <td>KiArgFl_Global         </td></tr>
 *  <tr><td>11</td> <td>KiArgFl_SubcmdsOptional</td></tr>
 *  <tr><td>12</td> <td>KiArgFl_Negatable      </td></tr>
 *  <tr><td>13</td> <td>KiArgFl_Disabled       </td></tr>
 * </table>
 */
static KiSStaticArray const *gl_c_CmdlArgFlagIncompatibilityMatrix[14 /* log2(KiArgFl_Disabled) + 1 */] = {
    /* log2(KiArgFl_Required)        */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 }),
    /* log2(KiArgFl_Optional)        */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 }),
    /* log2(KiArgFl_Deprecated)      */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }),
    /* log2(KiArgFl_NoHelp)          */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0 }),
    /* log2(KiArgFl_Switch)          */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0 }),
    /* log2(KiArgFl_InvSwitch)       */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0 }),
    /* log2(KiArgFl_List)            */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0 }),
    /* log2(KiArgFl_Countable)       */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0 }),
    /* log2(KiArgFl_BoundsBeginExcl) */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0 }),
    /* log2(KiArgFl_BoundsEndExcl)   */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0 }),
    /* log2(KiArgFl_Global)          */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 }),
    /* log2(KiArgFl_SubcmdsOptional) */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0 }),
    /* log2(KiArgFl_Negatable)       */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0 }),
    /* log2(KiArgFl_Disabled)        */ &KI_MAKE_STATIC_ARRAY((KiTUint64 []){ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
};


/**
 */
static KiTUint64 KI_CALL KiInternal_CmdlMakeIncMask(KiSStaticArray const *bmArr) {
    KI_ASSERT(bmArr != nullptr, KiErr_InParameter);

    KiTUint64 res = 0;
    {
        for (KiTSize i = 0; i < KI_MIN(64, bmArr->m_elemCount); i++)
            res |= ((KiTUint64 *)bmArr->mp_arrPtr)[i] << i;
    }

    return res;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlValidateArgumentSpec(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgument const *argPtr
) {
    KI_ASSERT(nsPtr != nullptr,  KiErr_InOutParameter);
    KI_ASSERT(argPtr != nullptr, KiErr_InParameter);

    // S -> GV | --IAV | -CV | I
    // G -> [--IA,-C] | [--IA] | [-C] | [-C,--IA]
    // V -> =I | e
    // I -> CI | C
    // A -> (X) | e
    // C -> a | b | c | d
    // X -> I,X | I
    // 

    KiEErrorCode                        errCode = KiErr_Ok;
    KiSCommandLineParsingToken          currTok = KI_ENV_NEWTOK;
    KiSCommandLineArgumentSpecification argSpec = KI_ENV_NEWSPEC;
    {
        /* Get first token and decide what to do. */
        KiTChar *currStrPtr = (KiTChar *)argPtr->mp_spec;
        /// TODO: validate string

        switch (KiInternal_CmdlSpecProduceToken(&currStrPtr, &currTok)->m_type) {
            case KiCmdlTokTy_ShortPrefix:
                /* No group, so we just parse the short argument specification. */
                errCode = KiInternal_CmdlSpecParseShortArg(nsPtr, &argSpec, &currStrPtr);

                break;
            case KiCmdlTokTy_LongPrefix:
                /* No group, just a long identifier with maybe a list of aliases. */
                errCode = KiInternal_CmdlSpecParseLongArg(nsPtr, &argSpec, &currStrPtr);

                break;
            case KiCmdlTokTy_BeginGroup:
                /* Ok, we got a group. Parse it, including the ']' token. */
                errCode = KiInternal_CmdlSpecParseGroup(nsPtr, &argSpec, &currStrPtr);

                break;
            case KiCmdlTokTy_Value:
                /*
                 * If it is simply an identifier, we have a sub-command. In this case, we simply add the identifier as
                 * the main identifier for later.
                 */
                errCode = KiInternal_CmdlSpecAddIdentifier(
                    nsPtr,
                    &argSpec,
                    KiCmdlArgNT_Main,
                    &(KiSStringView const){ .mp_strPtr = currTok.mp_startPtr, .m_sizeInBytes = currTok.m_len }
                );
                
                break;
            default:
                goto lbl_ONUNEXPTOK;
        }

        if (errCode != KiErr_Ok) {
            /* Add error. */
        }

        /* Do we have a metavalue? */
        if (KiInternal_CmdlSpecProduceToken(&currStrPtr, &currTok)->m_type == KiCmdlTokTy_KVSeparator) {
            /* Next token must be an identifier. */
            if (KiInternal_CmdlSpecProduceToken(&currStrPtr, &currTok)->m_type != KiCmdlTokTy_Value)
                goto lbl_ONUNEXPTOK;

            /* Store the metavar. */
            errCode = KiInternal_CmdlSpecAddIdentifier(nsPtr, &argSpec, KiCmdlArgNT_Metavar, &(KiSStringView const){
                .mp_strPtr     = currTok.mp_startPtr,
                .m_sizeInBytes = currTok.m_len
            });
        }
    }

    /* Return error. Even if there was an error, we might get here. */
    return errCode;

    /* Handle unexpected tokens. */
lbl_ONUNEXPTOK:
    /// TODO: unexpected token

    return KiErr_UnexpectedSpecToken;
}

/**
 * \brief  checks for errors in the command-line argument flag usage
 * \param  [in] argPtr pointer to the command-line argument being parsed
 * \return \c KiErr_Ok if all flags are compatible, non-zero if not
 * \note   Some flags are only meaningful alongside other semantic information. Checking semantic validity of the
 *         command-line argument's configuration is not done by this function. This function simply checks if there any
 *         of the command-line argument's flags are not meant to be used together.
 */
static KiEErrorCode KI_CALL KiInternal_CmdlValidateArgumentFlags(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgument const *argPtr
) {
    KI_UNREFERENCED_PARAMETER(nsPtr);

    KiECommandLineArgumentFlags const argFls = argPtr->m_flags;
    {
        for (KiTInt32 mask = 0x01; argFls ^ KiArgFl_None && mask < KiArgFl_Negatable << 1; mask <<= 1) {
            KiTUint64 const incompMask = KiInternal_CmdlMakeIncMask(gl_c_CmdlArgFlagIncompatibilityMatrix[KI_P2LOG2(mask)]);

            if (argFls & mask ^ KiArgFl_None && argFls & incompMask ^ KiArgFl_None) {
                /// TODO: err: invalid flag combo

                return KiErr_InvalidFlagCombination;
            }
        }
    }

    /* We went through all flags. Perfectly balanced, as all things should be. */
    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlValidateSchemaRoot(KiSCommandLineSchema const *cmdlSchemaPtr) {
    if (cmdlSchemaPtr == nullptr)
        return KiErr_InParameter;

    /* (1) 'name' and 'desc' must not be empty. */
    KiTBool const isNameValid = cmdlSchemaPtr->mp_name != nullptr && *cmdlSchemaPtr->mp_name ^ '\0';
    KiTBool const isDescValid = cmdlSchemaPtr->mp_desc != nullptr && *cmdlSchemaPtr->mp_desc ^ '\0';
    {
        return isNameValid && isDescValid ? KiErr_Ok : KiErr_ReqPropNotProvided;
    }
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlValidateNode(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineArgument const *cmdlArgPtr
) {
    KiEErrorCode errCode = KiErr_Ok;
    {
        /** \cond */
        static KiFCommandLineValidationRoutine const gl_c_ValidationPasses[] = {
            &KiInternal_CmdlValidateArgumentSpec,
            &KiInternal_CmdlValidateArgumentFlags
        };
        /** \endcond */

        for (KiTSize i = 0; i < KI_COUNTOF(gl_c_ValidationPasses); i++) {
            errCode = (*gl_c_ValidationPasses[i])(nsPtr, cmdlArgPtr);

            if (errCode != KiErr_Ok)
                return errCode;
        }
    }

    /* All good. */
    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlValidateCommandLineSchema(
    KiSCommandLineNamespace *nsPtr,
    KiSCommandLineSchema const *cmdlSchemaPtr
) {
    if (cmdlSchemaPtr == nullptr)
        return KiErr_InParameter;
    
    /* We first validate the root, then recursively validate all nodes. */
    KiEErrorCode validationRes = KiInternal_CmdlValidateSchemaRoot(cmdlSchemaPtr);
    if (validationRes != KiErr_Ok)
        return validationRes;

    /* Now, we recursively validate the child arguments if they exist. */
    //for (KiTSize i = 0; i < (cmdlSchemaPtr->mp_args != nullptr ? cmdlSchemaPtr->mp_args->m_elemCount : 0); i++) {
    //    KiSCommandLineArgument const *const currArg = ((KiSCommandLineArgument **)cmdlSchemaPtr->mp_args->mp_arrPtr)[i];
//
    //    if ((validationRes = KiInternal_CmdlValidateNode(nsPtr, currArg)) != KiErr_Ok)
    //        return validationRes;
    //}

    /* All good. */
    return KiErr_Ok;
}
#pragma endregion


#pragma region Namespace-Mngt
/**
 */
static KiEErrorCode KI_CALL KiInternal_CmdlCreateNamespace(
    KiSCommandLineSchema const *cmdlSchemaPtr,
    KiSCommandLineNamespace **resPtr
) {
    if (cmdlSchemaPtr == nullptr) return KiErr_InParameter;
    if (resPtr == nullptr)        return KiErr_OutptrParameter;

    /* Create the namespace object. Right now, the schema is not yet validated. */
    if ((*resPtr = malloc(sizeof **resPtr)) == nullptr)
        return KiErr_MemoryAllocation;

    /* Create string buffer. */
    KiSBuffer *strBuf;
    KiEErrorCode errCode = KiCreateBuffer(0, &strBuf);
    if (errCode != KiErr_Ok) {
        free(*resPtr);

        *resPtr = nullptr;
        return KiErr_MemoryAllocation;
    }

    /* Create argument hashtable. */
    KiSHashtable *hTable;
    if ((errCode = KiCreateHashtable(32, nullptr, nullptr, &hTable)) != KiErr_Ok) {
        KiDestroyBuffer(strBuf);

        free(*resPtr);
        *resPtr = nullptr;
        return KiErr_MemoryAllocation;
    }

    **resPtr = (KiSCommandLineNamespace const){
        .mp_schema = (KiSCommandLineSchema *)cmdlSchemaPtr,
        .mp_strs   = strBuf,
        .mp_args   = hTable
    };
    return KiErr_Ok;
}

/**
 */
static KiTVoid KI_CALL KiInternal_CmdlDestroyNamespace(KiSCommandLineNamespace *nsPtr) {
    if (nsPtr != nullptr) {
        KiDestroyString(nsPtr->mp_path);
        KiDestroyHashtable(nsPtr->mp_args);

        KiDestroyBuffer(nsPtr->mp_strs);
        KiDestroyBuffer(nsPtr->mp_diags);
        KiDestroyBuffer(nsPtr->mp_specs);
    }

    free(nsPtr);
}
#pragma endregion
/** \endcond */


KiSCommandLineNamespace KI_CALL *KiParseCommandLine(KiSCommandLineSchema const *cmdlSchemaPtr, int argc, char **argv) {
    /* Create namespace object. */
    KiSCommandLineNamespace *nsObj;
    {
        KiEErrorCode const errCode = KiInternal_CmdlCreateNamespace(cmdlSchemaPtr, &nsObj);

        if (nsObj == nullptr)
            return nullptr;
    }
    /* Before we can parse the command-line, we must validate our schema. */
    KiEErrorCode schValRes = KiInternal_CmdlValidateCommandLineSchema(nsObj, cmdlSchemaPtr);
    if (schValRes != KiErr_Ok) {

        return nsObj;
    }

    /* Now we actually parse the command-line arguments. */
    // ...

    /* All good. */
    return nsObj;
}

KiTVoid KI_CALL KiCleanupCommandLine(KiSCommandLineNamespace *nsPtr) {
    if (nsPtr == nullptr)
        return;

    KiInternal_CmdlDestroyNamespace(nsPtr);
}


KiSArrayView const KI_CALL KiGetCommandLineDiagnostics(KiSCommandLineNamespace const *nsPtr) {
    KI_ASSERT(nsPtr != nullptr, KiErr_InParameter);

    return (KiSArrayView const){
        .mp_arrPtr   = KiGetBufferPointer((KiSBuffer const *)nsPtr->mp_diags, 0),
        .m_elemSize  = sizeof(KiSCommandLineDiagnostics),
        .m_elemCount = KiGetBufferPosition((KiSBuffer const *)nsPtr->mp_diags) / sizeof(KiSCommandLineDiagnostics)
    };
}

KiSCommandLineSchema const *KI_CALL KiGetCommandLineSchema(KiSCommandLineNamespace const *nsPtr) {
    KI_ASSERT(nsPtr != nullptr, KiErr_InParameter);

    return nsPtr->mp_schema;
}

KiSVariant KI_CALL KiGetCommandLineArgument(KiSCommandLineNamespace const *nsPtr, KiTChar const *aName) {
    KI_ASSERT(nsPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(aName != nullptr, KiErr_InParameter);
    KI_ASSERT(*aName != '\0',   KiErr_InParameter);

    KiSVariant const *valPtr = KiGetFromHashtable(nsPtr->mp_args, aName);
    {
        if (valPtr == nullptr)
            return (KiSVariant){ KiVarTy_Invalid };
    }

    return *valPtr;
}

KiTSize KI_CALL KiGetCommandLineSubArgumentLimit(KiTVoid) {
    return KI_ENV_MAXSUBARGS;
}

KiTSize KI_CALL KiGetCommandLineArgumentChoicesLimit(KiTVoid) {
    return KI_ENV_MAXCHOICES;
}


