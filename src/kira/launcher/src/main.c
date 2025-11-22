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
/**
 */
KI_COMMANDLINE(gl_c_CommandLine) {
    KI_NAME   "kira",
    KI_DESC   "default run-time for Kira applications",
    KI_PREFIX "-/",
    KI_SEP    "=:",

    KI_ARGUMENTS ({
        KI_ARGUMENT { KI_TYPE KiArgTy_String, KI_SPEC "--config-file", KI_DESC "config file to use"                                      },
        KI_ARGUMENT { KI_TYPE KiArgTy_String, KI_SPEC "--use-profile", KI_DESC "profile to load"                                         },
        KI_ARGUMENT { KI_TYPE KiArgTy_String, KI_SPEC "--module-path", KI_DESC "module path to load modules from", KI_FLAGS KiArgFl_List },

        KI_SUBCOMMAND {
            KI_SPEC  "run",
            KI_FLAGS KiArgFl_Required,

            KI_ARGUMENTS ({
                KI_SUBCOMMAND {
                    KI_SPEC  "run",
                    KI_FLAGS KiArgFl_Required,

                    KI_ARGUMENTS ({
                        KI_ARGUMENT { KI_TYPE KiArgTy_String, KI_SPEC "--config-file", KI_DESC "config file to use"                                      },
                        KI_ARGUMENT { KI_TYPE KiArgTy_String, KI_SPEC "--use-profile", KI_DESC "profile to load"                                         },
                        KI_ARGUMENT { KI_TYPE KiArgTy_String, KI_SPEC "--module-path", KI_DESC "module path to load modules from", KI_FLAGS KiArgFl_List },
                    })
                }
            })
        },
        KI_SUBCOMMAND {
            KI_SPEC "test",

            KI_ARGUMENTS ({})
        }
    })
};


/**
 */
int main(int argc, char **argv, char **envp) {
    KiSReturnState retState;

    KiParseCommandLine(&gl_c_CommandLine, argc, argv);
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


