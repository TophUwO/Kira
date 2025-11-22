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
 * \file  algo.h
 * \brief defines the APIs for abstract algorithmic patterns (sorting, enumerating, etc.)
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/**
 */
KI_INTERFACE(KiIEnumerator) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    /**
     * \brief reimplements \c KiIBase::QueryInterface()
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIEnumerator *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     * \brief reimplements \c KiIBase::Acquire()
     */
    KiTInt32 (KI_CALL *Acquire)(KiIEnumerator *self);
    /**
     * \brief reimplements \c KiIBase::Release
     */
    KiTInt32 (KI_CALL *Release)(KiIEnumerator *self);

    /**
     */
    KiEErrorCode (KI_CALL *GetLastError)(KiIEnumerator *self);

    /**
     */
    KiTVoid *(KI_CALL *Yield)(KiIEnumerator *self);
    /**
     */
    KiTVoid (KI_CALL *Discard)(KiIEnumerator *self, KiTVoid *resPtr);
    /**
     */
    KiTVoid (KI_CALL *Reset)(KiIEnumerator *self);
};

/**
 */
KI_INTERFACE(KiIAggregator) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    /**
     * \brief reimplements \c KiIBase::QueryInterface()
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIAggregator *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     * \brief reimplements \c KiIBase::Acquire()
     */
    KiTInt32 (KI_CALL *Acquire)(KiIAggregator *self);
    /**
     * \brief reimplements \c KiIBase::Release
     */
    KiTInt32 (KI_CALL *Release)(KiIAggregator *self);

    /**
     */
    KiEErrorCode (KI_CALL *Aggregate)(KiIAggregator *self, KiTVoid const *initValPtr, KiTVoid *resPtr);
};

/**
 */
KI_INTERFACE(KiIFilter) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    /**
     * \brief reimplements \c KiIBase::QueryInterface()
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIFilter *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     * \brief reimplements \c KiIBase::Acquire()
     */
    KiTInt32 (KI_CALL *Acquire)(KiIFilter *self);
    /**
     * \brief reimplements \c KiIBase::Release
     */
    KiTInt32 (KI_CALL *Release)(KiIFilter *self);

    /**
     */
    KiTBool (KI_CALL *Test)(KiIFilter *self, KiTVoid const *elemToTest, KiTVoid *extraParam);
};

/**
 */
KI_INTERFACE(KiIMapping) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    /**
     * \brief reimplements \c KiIBase::QueryInterface()
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIMapping *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     * \brief reimplements \c KiIBase::Acquire()
     */
    KiTInt32 (KI_CALL *Acquire)(KiIMapping *self);
    /**
     * \brief reimplements \c KiIBase::Release
     */
    KiTInt32 (KI_CALL *Release)(KiIMapping *self);

    /**
     */
    KiTVoid *(KI_CALL *Map)(KiIMapping *self, KiTVoid const *srcVal, KiTVoid *dstVal);
};


