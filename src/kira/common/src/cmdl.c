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
    KiCmdlAC_Invalid    = 0,

    KiCmdlAC_Positional = 1 << 0,
    KiCmdlAC_Option     = 1 << 1,
    KiCmdlAC_SubCommand = 1 << 2,

    KiCmdlAC_All        = KiCmdlAC_Positional | KiCmdlAC_Option | KiCmdlAC_SubCommand,
    KiCmdlAC_Args       = KiCmdlAC_Positional | KiCmdlAC_Option,

    KI_ENUM_GEN_LAST(__KiCmdlAC_Last__, KiCmdlAC_SubCommand)
} KiECommandLineArgumentCategory;

 
/**
 */
KI_NATIVE typedef struct KiSCommandLineValidationPass {
    KiSStringView                   m_passName;
    KiFCommandLineValidationRoutine m_routine;
} KiSCommandLineValidationPass;

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
    if (argPtr == nullptr || argPtr->mp_spec == nullptr || *argPtr->mp_spec == '\0')
        return KiCmdlAC_Invalid;

    /*
     * If first two chars are '--' or the first char is ';', we assume it's an option. This is safe to check because we
     * already checked if the string is at least two bytes in total (which is the case if *argPtr->mp_spec == '\0' is
     * false.)
     */
    if (strncmp(argPtr->mp_spec, "--", 2) == 0 || *argPtr->mp_spec == ';')
        return KiCmdlAC_Option;

    /*
     * Otherwise, if we have child arguments, we assume it's a sub-command or, if not, a positional argument.
     * Positionals cannot start with a '-' or any other character in KI_SEP (if present), but are accepted here. In such
     * a case, an error would be thrown when validating the schema.
     */
    return argPtr->mp_args != nullptr ? KiCmdlAC_SubCommand : KiCmdlAC_Positional;
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
        static KiSCommandLineValidationPass constexpr gl_c_ValidationPasses[] = {
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


#pragma region Namespace-Mngt
/**
 */
static KiSCommandLineNamespace *KI_CALL KiInternal_CmdlCreateNamespace(KiSCommandLineSchema const *cmdlSchemaPtr) {
    return malloc(sizeof(KiSCommandLineNamespace));
}

/**
 */
static KiTVoid KI_CALL KiInternal_CmdlDestroyNamespace(KiSCommandLineNamespace *nsPtr) {

}
#pragma endregion
/** \endcond */


KiSCommandLineNamespace KI_CALL *KiParseCommandLine(KiSCommandLineSchema const *cmdlSchemaPtr, int argc, char **argv) {
    /* Create namespace object. */
    KiSCommandLineNamespace *nsPtr = KiInternal_CmdlCreateNamespace(cmdlSchemaPtr);
    if (nsPtr == nullptr)
        return nullptr;

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
    if (nsPtr == nullptr)
        return KiErr_InParameter;

    return nsPtr->m_errCode;
}

KiSCommandLineSchema const *KI_CALL KiGetCommandLineSchema(KiSCommandLineNamespace const *nsPtr) {
    if (nsPtr == nullptr)
        return nullptr;

    return nsPtr->mp_schemaPtr;
}

KiSVariant KI_CALL KiGetCommandLineArgument(KiSCommandLineNamespace const *nsPtr, KiTChar const *aName) {
    if (nsPtr == nullptr || aName == nullptr || *aName == '\0')
        return (KiSVariant){ KiVarTy_Null };

    /* Get the value. */

}


