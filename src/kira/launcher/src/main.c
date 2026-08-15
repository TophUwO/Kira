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
 * \file  main.c
 * \brief defines the entry point of Kira's runtime component
 */


/* Kira includes */
#include <kira/kernel/rt.h>


/** \cond INTERNAL */
/**
 */
int main(int argc, char **argv, char **envp) {
    KiSReturnState retState;

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

        /* Start kernel. This loads the kernel and all modules. */
        KiEErrorCode errCode = KiStartup(&(KiSRuntimeSpecification const){
           .m_structSize = sizeof(KiSRuntimeSpecification),
           .m_argc       = argc,
           .mpp_argv     = argv,
           .mpp_envp     = envp
        });
        if (errCode != KiErr_Ok) {
           retState.m_errCode = errCode;

           KiShutdown();
           break;
        }

        /*
        * Start main-loop and run application. If the error code is KiErr_Ok and the restart flag is set to KI_TRUE,
        * we will soft-'restart' the application.
        */
        KiRun(&retState);

        /* Shutdown kernel. This also unloads all modules. */
        KiShutdown();
    } while (retState.m_wantsRestart && retState.m_errCode == KiErr_Ok);

    return (int)retState.m_errCode;
}
/** \endcond */
