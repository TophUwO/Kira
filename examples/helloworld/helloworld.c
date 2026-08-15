/****************************************************************************************************************
 * Kira - cross-platform component-based modular application development framework written in C11               *
 *                                                                                                              *
 * (c) 2024-2026 Toffi <tophuwo01@gmail.com>                                                                    *
 *                                                                                                              *
 * The source code is licensed under the Apache License 2.0. Refer to the LICENSE file in the root directory of *
 * this project. If this file is not present, visit                                                             *
 *     https://www.apache.org/licenses/LICENSE-2.0                                                              *
 ****************************************************************************************************************/

/**
 * \file   helloworld.c
 * \author Toffi <tophuwo01@gmail.com>
 * \date   August 15, 2026
 * \brief  demonstrates how to create an absolutely minimal application with Kira
 */


/* stdlib includes */
#include <stdio.h> /* imports puts() */

/* Kira includes */
#include <kira/app.h> /* imports the KiIApplication interface that serves as the entrypoint */


/**
 * \class KiCHelloWorldApplication
 * \brief component that implements the \c KiIApplication interface which exposes the entrypoint of a Kira application
 */
KI_COMPONENT(KiCHelloWorldApplication) {
    /**
     * \brief declares that the \c KiCHelloWorldApplication component implements the \c KiIApplication interface; it
     *        exposes the \c Run() method which will be called by the kernel and works analogously to the standard
     *        \c main() function in a normal C program.
     */
    KI_IMPLEMENTS(KiIApplication); 
};


/**
 * \fn    KiCHelloWorldApplication_KiIApplication_Run(KiIApplication *, KiSReturnState *)
 * \brief implements the \c Run() method of the \c KiIApplication interface which is implemented by the \c KiCHelloWorldApplication
 *        component
 * \param [in, out] self pointer to the \c KiIApplication instance that this method operates on
 * \param [in, out] resPtr pointer to a data structure of type \c KiSReturnState which has to be written the result code to
 */
KiTVoid KI_CALL KiCHelloWorldApplication_KiIApplication_Run(KiIApplication *self, KiSReturnState *resPtr) {
    /* Just output a message. */
    puts("Hello, world!");

    /*
     * By default, the result structure is initialized to signify a non-error state when this method returns. Thus, it
     * is technically not necessary to write to "resPtr" if no error occurs. Still, it is good practice to always do it.
     * It is omitted in this example for the sake of brevity.
     */
    return;
}
