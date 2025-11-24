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
 * \file  main.c
 * \brief defines the entry point of Kira's runtime component
 */


/* Kira includes */
#include <kira/cmdl.h>

#include <kira/kernel/rt.h>


/** \cond INTERNAL */
KiEErrorCode KI_CALL CheckDigestSize(KiSCommandLineArgument const *aPtr, KiSVariant const *vPtr) {
    // Define the array of possible values.
    KiSStaticArray const arr = KI_MAKE_STATIC_ARRAY((KiTInt64 []){ 1, 2, 4, 8, 16, 32, 64, 128, 256, 512 });
    if (vPtr->m_type != KiVarTy_Int64) {
        // Return 'KiErr_InvalidArgType' if the argument type is incorrect.
        return KiErr_InvalidArgType;
    }

    // Check if the parsed value (vPtr) is in this array.
    for (KiTSize i = 0; i < arr.m_elemCount; i++)
        if (vPtr->m_i64Val == ((KiTInt64 *)arr.mp_arrPtr)[i]) {
            // Return 'KiErr_Ok' if a match has been found.
            return KiErr_Ok;
        }
    
    // Return 'KiErr_InvalidArgValue' if the parsed value is not valid.
    return KiErr_InvalidArgValue;
}

KiTVoid KI_CALL ProcDigestSize(KiSCommandLineArgument const *aPtr, KiSVariant *vPtr) {
    // Function that can be used to do post-processing on the actual value (vPtr).
    // You are allowed to modify the value in vPtr; the changes you make to vPtr
    // will be reflected when the argument is queried via \c KiGetCommandLineArgument().
}
    
static KI_COMMANDLINE(gl_c_MultiFunction) {
    KI_NAME   "kitool",
    KI_DESC   "example command line schema implementing a single devtool for generating hashes and uuids",
    KI_PREFIX "-/",
    KI_SEP    "=:",

    KI_ARGUMENTS({
        // global optional arguments
        KI_ARGUMENT { KI_TYPE KiArgTy_Integer, KI_SPEC "--verbose;v", KI_DESC "enable verbose mode",      KI_FLAGS KiArgFl_Countable },
        KI_ARGUMENT { KI_TYPE KiArgTy_Bool,    KI_SPEC "--copy;c",    KI_DESC "copy result to clipboard", KI_FLAGS KiArgFl_Switch    },
        
        // sub-commands
        KI_SUBCOMMAND {
            KI_SPEC "hash",
            KI_DESC "generate SHA-512 hash of any input string",
            
            // arguments specific to the 'hash' sub-command
            KI_ARGUMENTS({
                KI_ARGUMENT { KI_TYPE KiArgTy_String, KI_SPEC "input", KI_DESC "the input string", KI_FLAGS KiArgFl_Required },
                KI_ARGUMENT {
                    KI_TYPE    KiArgTy_Integer,
                    KI_SPEC    "--size;s",
                    KI_DESC    "size of digest to return",
                    KI_CHECK   CheckDigestSize,
                    KI_PROC    ProcDigestSize,
                    KI_DEFAULT KI_INT(512),
                    KI_METAVAR "SIZE"
                }
            })
        },
        KI_SUBCOMMAND {
            KI_SPEC  "uuid",
            KI_DESC  "generate version-4 UUIDs",
            KI_FLAGS KiArgFl_NoHelp | KiArgFl_Deprecated,
            
            // no arguments
            KI_ARGUMENTS({})
        },
        KI_SUBCOMMAND {
            KI_SPEC "range",
            KI_DESC "checks a numeric range for validity",
            
            KI_ARGUMENTS({
                KI_ARGUMENT {
                    KI_TYPE    KiArgTy_NumericRange,
                    KI_SPEC    "in",
                    KI_DESC    "input range",
                    KI_FLAGS   KiArgFl_Optional,
                    KI_DEFAULT KI_RANGE(1.0, 2.0)
                },
                KI_ARGUMENT {
                    KI_TYPE    KiArgTy_Float,
                    KI_SPEC    "x",
                    KI_DESC    "example float value",
                    KI_DEFAULT KI_FLOAT(0.0f),
                    KI_BOUNDS  (-2.f, 2.f)
                },
                KI_ARGUMENT {
                    KI_TYPE    KiArgTy_String,
                    KI_SPEC    "--output-fmt;o",
                    KI_DESC    "output string format",
                    KI_DEFAULT KI_STRING("blake"),
                    KI_ENUM    ({ KI_STRING("sha1"), KI_STRING("sha256"), KI_STRING("sha384"), KI_STRING("blake3") })
                }
            })
        }
    })
};


/**
 */
int main(int argc, char **argv, char **envp) {
    KiSReturnState retState;

    KiParseCommandLine(&gl_c_MultiFunction, argc, argv);
    return 0;

    /*
     * Since we want to support a full application restart without actually restarting, we want to check whether we need
     * to quit when KiKrnlRun() returns.
     */
    do {
        /* Initialize runtime return state. */
        retState = (KiSReturnState){
            .m_errCode      = KiErr_Ok,
            .m_wantsRestart = KI_FALSE
        };

        ///* Start kernel. This loads the kernel and all modules. */
        //KiEErrorCode errCode = KiStartup(&(KiSRuntimeSpecification const){
        //    .m_structSize = sizeof(KiSRuntimeSpecification),
        //    .m_argc       = argc,
        //    .mpp_argv     = argv,
        //    .mpp_envp     = envp,
        //    .mp_dbgOpts   = nullptr
        //});
        //if (errCode != KiErr_Ok) {
        //    retState.m_errCode = errCode;
//
        //    KiShutdown();
        //    break;
        //}
//
        ///*
        // * Start main-loop and run application. If the error code is KiErr_Ok and the restart flag is set to KI_TRUE,
        // * we will soft-'restart' the application.
        // */
        //KiRun(&retState);
//
        ///* Shutdown kernel. This also unloads all modules. */
        //KiShutdown();
    } while (retState.m_wantsRestart && retState.m_errCode == KiErr_Ok);

    return (int)retState.m_errCode;
}
/** \endcond */


