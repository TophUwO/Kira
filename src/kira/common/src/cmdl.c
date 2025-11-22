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
 * \file  cmdl.c
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
    //  (u) flag grouping (e.g., -vsx for -v -s -x) is not allowed unless all flags in the group take no value (are countable, switch or invswitch)
    //  (v) globals must be unique in their name; cannot appear anywhere in any other sub-tree
    //  (x) globals must appear only at the root scope
    //  (y) if a global is given after a sub-command, then global scope is searched and matched (globals are highest class citizens)

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Kira includes */
#include <kira/dbg.h>
#include <kira/cmdl.h>

#include <kira/common/buffer.h>


/** \cond INTERNAL */
/**
 */
KI_NATIVE typedef KiEErrorCode (KI_CALL *KiFCommandLineValidationRoutine)(KiSCommandLineArgument const *argPtr);


/**
 */
KI_NATIVE typedef enum KiECommandLineArgumentCategory {
    KiCmdlArgCat_Invalid = 0,

    KiCmdlArgCat_Positional,
    KiCmdlArgCat_Option,
    KiCmdlArgCat_SubCommand,

    __KiCmdlArgCat_Count__
} KiECommandLineArgumentCategory;

 
/**
 */
KI_NATIVE typedef struct KiSCommandLineValidationPass {
    KiSStringView                   m_passName;
    KiFCommandLineValidationRoutine m_routine;
} KiSCommandLineValidationPass;

/**
 */
KI_NATIVE typedef struct KiSExpandedCommandLineArgument {

} KiSExpandedCommandLineArgument;

/**
 */
KI_NATIVE typedef struct KiSCommandLineNamespace {
    KiSCommandLineSchema const *mp_schemaPtr;
    KiSBuffer                  *mp_strBuffer;
   // KiSHashtable               *mp_args;
    KiEErrorCode                m_errCode;
} KiSCommandLineNamespace;


#pragma region Schema-Utils
/**
 */
static KiECommandLineArgumentCategory KI_CALL KiInternal_CmdlGetArgumentCategory(KiSCommandLineArgument const *argPtr) {
    if (argPtr == nullptr)
        return KiCmdlArgCat_Invalid;

    /* If first two chars are '--' or the first char is ';', we assume it's an option. */
    if (strlen(argPtr->mp_spec) >= 2 && *argPtr->mp_spec == '-' && argPtr->mp_spec[1] == '-' || *argPtr->mp_spec == ';')
        return KiCmdlArgCat_Option;

    /*
     * Otherwise, if we have child arguments, we assume it's a sub-command or, if not, a positional argument.
     * Positionals cannot start with a '-' or any other character in KI_SEP (if present), but are accepted here. In such
     * a case, an error would be thrown when validating the schema.
     */
    return argPtr->mp_args != nullptr ? KiCmdlArgCat_SubCommand : KiCmdlArgCat_Positional;
}
#pragma endregion


#pragma region Schema-Printing
/**
 */
static KiTChar const *KI_CALL KiInternal_CmdlMakeIndent(KiTInt32 lvl) {
    /**
     */
    static KiTSize constexpr gl_c_TabSize = 4;
     
    static KiTSize _Thread_local gl_IndentSize;
    static KiTChar _Thread_local gl_IndentBuffer[256];
    {
        /* Clear the old indent. */
        memset(gl_IndentBuffer, 0, gl_IndentSize);

        /* Write the new indent. */
        for (KiTSize i = 0; i < KI_MIN((KiTSize)lvl, sizeof gl_IndentBuffer / gl_c_TabSize); i++)
            strcat_s(gl_IndentBuffer, sizeof gl_IndentBuffer, "    ");
        gl_IndentSize = lvl * gl_c_TabSize;
    }

    return gl_IndentBuffer;
}

/**
 */
static KiTChar const *KI_CALL KiInternal_CmdlMakeSchemaFlags(KiECommandLineSchemaFlags flags) {
    static KiTSize _Thread_local gl_IndentSize;
    static KiTChar _Thread_local gl_IndentBuffer[256];

    static KiECommandLineSchemaFlags const gl_c_CmdlSchemaColl[] = {
        
    };
}

/**
 */
static KiTChar const *KI_CALL KiInternal_CmdlMakeArgumentCategoryStr(KiSCommandLineArgument const *argPtr) {
    /**
     */
    static KiTChar constexpr *gl_c_CmdlArgCategoryStrings[] = {
        [KiCmdlArgCat_Invalid]    = "<error-type>",

        [KiCmdlArgCat_Positional] = "POSITIONAL",
        [KiCmdlArgCat_Option]     = "OPTION",
        [KiCmdlArgCat_SubCommand] = "SUBCOMMAND"
    };
    KI_VERIFY_LUT(gl_c_CmdlArgCategoryStrings, __KiCmdlArgCat_Count__);

    /*
     * KiInternal_CmdlGetArgumentCategory() only returns values that are part of the KiECommandLineArgumentCategory
     * enumeration.
     */
    return gl_c_CmdlArgCategoryStrings[KiInternal_CmdlGetArgumentCategory(argPtr)];
}


/**
 */
static KiTVoid KI_CALL KiInternal_CmdlPrintArgumentSpec(KiTChar const *rawSpecStr) {

}

/**
 */
static KiTVoid KI_CALL KiInternal_CmdlPrintCommandLineArgument(KiSCommandLineArgument const *argPtr, KiTInt32 lvl) {
    printf("%s%s %s {\n", KiInternal_CmdlMakeIndent(lvl), KiInternal_CmdlMakeArgumentCategoryStr(argPtr), argPtr->mp_spec);
    {
        KiInternal_CmdlPrintArgumentSpec(argPtr->mp_spec);

        /* If we have child arguments, we print them. */
        if (argPtr->mp_args != nullptr) {
            /** \cond */
            extern KiTVoid KI_CALL KiInternal_CmdlPrintCommandLineArguments(KiSStaticArray const *argArr, KiTInt32 lvl);
            /** \endcond */

            KiInternal_CmdlPrintCommandLineArguments(argPtr->mp_args, lvl + 1);
        }
    }
    printf("%s}\n", KiInternal_CmdlMakeIndent(lvl));
}

/**
 */
static KiTVoid KI_CALL KiInternal_CmdlPrintCommandLineArguments(KiSStaticArray const *argArr, KiTInt32 lvl) {
    printf("\n%sARGUMENTS {\n", KiInternal_CmdlMakeIndent(lvl));
    {
        for (KiTSize i = 0; i < argArr->m_elemCount; i++)
            KiInternal_CmdlPrintCommandLineArgument(((KiSCommandLineArgument const **)argArr->mp_arrPtr)[i], lvl + 1);
    }
    printf("%s}\n", KiInternal_CmdlMakeIndent(lvl));
}

/**
 */
static KiTVoid KI_CALL KiInternal_CmdlPrintCommandLineSchema(KiSCommandLineSchema const *cmdlSchemaPtr) {
    printf("SCHEMA %s {\n", cmdlSchemaPtr->mp_name);
    {
        printf("%sPROPERTY name       \"%s\"\n", KiInternal_CmdlMakeIndent(1), cmdlSchemaPtr->mp_name);
        printf("%sPROPERTY desc       \"%s\"\n", KiInternal_CmdlMakeIndent(1), cmdlSchemaPtr->mp_desc);
        printf("%sPROPERTY flags       %s\n",    KiInternal_CmdlMakeIndent(1), "");
        printf("%sPROPERTY prefixes   \"%s\"\n", KiInternal_CmdlMakeIndent(1), cmdlSchemaPtr->mp_prefixes);
        printf("%sPROPERTY separators \"%s\"\n", KiInternal_CmdlMakeIndent(1), cmdlSchemaPtr->mp_seps);

        KiInternal_CmdlPrintCommandLineArguments(cmdlSchemaPtr->mp_args, 1);
    }
    printf("}\n");
}
#pragma endregion


#pragma region Schema-Validation
/**
 */
static KiEErrorCode KI_CALL KiInternal_ValidateSpec(KiSCommandLineArgument const *argPtr) {

}

/**
 */
static KiEErrorCode KI_CALL KiInternal_ValidateRoot(KiSCommandLineSchema const *cmdlSchemaPtr) {
    /* (1) 'name' and 'desc' must not be empty. */
    KiTBool const isNameValid = cmdlSchemaPtr->mp_name != nullptr && *cmdlSchemaPtr->mp_name ^ '\0';
    KiTBool const isDescValid = cmdlSchemaPtr->mp_desc != nullptr && *cmdlSchemaPtr->mp_desc ^ '\0';
    {
        return isNameValid && isDescValid ? KiErr_Ok : KiErr_ReqPropNotProvided;
    }
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_ValidateNode(
    KiSCommandLineArgument const *cmdlArgPtr,
    KiSCommandLineNamespace const *nsPtr
) {
    KiEErrorCode errCode = KiErr_Ok;
    {
        /**
         */
        static KiSCommandLineValidationPass const gl_c_ValidationPasses[] = {
            { KI_MAKE_STRING_VIEW("ValidateSpec"), &KiInternal_ValidateSpec }
        };

        for (KiTSize i = 0; i < KI_COUNTOF(gl_c_ValidationPasses); i++) {
            errCode = (*gl_c_ValidationPasses[i].m_routine)(cmdlArgPtr);

            if (errCode != KiErr_Ok) {
                printf("%s: Error validating command-line schema.", nsPtr->mp_schemaPtr->mp_name);

                return errCode;
            }
        }
    }

    /* All good. */
    return KiErr_Ok;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_ValidateCommandLineSchema(
    KiSCommandLineSchema const *cmdlSchemaPtr,
    KiSCommandLineNamespace const *nsPtr
) {
    /* We first validate the root, then recursively validate all nodes. */
    KiEErrorCode validationRes = KiInternal_ValidateRoot(cmdlSchemaPtr);
    if (validationRes != KiErr_Ok)
        return validationRes;

    /* Now, we recursively validate the child arguments. */
    //if (cmdlSchemaPtr->mpp_args != nullptr)
    //    for (KiSCommandLineArgument const *currArg = cmdlSchemaPtr->mpp_args[0]; currArg != nullptr;) {
    //        validationRes = KiInternal_ValidateNode(currArg, nsPtr);
//
    //        if (validationRes != KiErr_Ok)
    //            return validationRes;
    //    }

    /* All good. */
    return KiErr_Ok;
}
#pragma endregion


/**
 */
static KiSCommandLineNamespace *KI_CALL KiInternal_CmdlCreateNamespace(KiSCommandLineSchema const *cmdlSchemaPtr) {
    return malloc(sizeof(KiSCommandLineNamespace));
}

/**
 */
static KiTVoid KI_CALL KiInternal_CmdlDestroyNamespace(KiSCommandLineNamespace *nsPtr) {

}
/** \endcond */


KiSCommandLineNamespace KI_CALL *KiParseCommandLine(KiSCommandLineSchema const *cmdlSchemaPtr, int argc, char **argv) {
    /* Create namespace object. */
    KiSCommandLineNamespace *nsPtr = KiInternal_CmdlCreateNamespace(cmdlSchemaPtr);
    if (nsPtr == nullptr)
        return nullptr;

    KiInternal_CmdlPrintCommandLineSchema(cmdlSchemaPtr);

    /* Before we can parse the command-line, we must validate our schema. */
    KiEErrorCode schValRes = KiInternal_ValidateCommandLineSchema(cmdlSchemaPtr, nsPtr);
    if (schValRes != KiErr_Ok) {
        KiInternal_CmdlDestroyNamespace(nsPtr);

        return nullptr;
    }

    /* All good. */
    return nsPtr;
}

KiTVoid KI_CALL KiCleanupCommandLine(KiSCommandLineNamespace *nsPtr) {
    if (nsPtr == nullptr)
        return;

    KiInternal_CmdlDestroyNamespace(nsPtr);
}


KiEErrorCode KI_CALL KiGetCommandLineError(KiSCommandLineNamespace const *nsPtr) {

}

KiSCommandLineSchema const *KI_CALL KiGetCommandLineSchema(KiSCommandLineNamespace const *nsPtr) {

}

KiSVariant KI_CALL KiGetCommandLineArgument(KiSCommandLineNamespace const *nsPtr, KiTChar const *aName) {

}


