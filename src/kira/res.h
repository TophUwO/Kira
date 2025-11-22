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
 * \file  kres.h
 * \brief defines the API for the <em>Kira Resource System</em> (KiRes)
 */


#pragma once

/* Kira includes */
#include <kira/kcm.h>


/** \cond */
KI_NATIVE typedef struct KiIStaticResourceLocator KiIStaticResourceLocator;
/** \endcond */


/**
 */
KI_NATIVE typedef KiIStaticResourceLocator *(KI_CALL *KiFQueryStaticResourceLocator)(KiTVoid);


/**
 */
KI_NATIVE typedef struct KiSStaticResourceMetadata {
    KiTSize       m_structSize;
    KiTUuid       m_uuid;
    KiSStringView m_path;
    KiSStringView m_prefix;
    KiSStringView m_name;
    KiSStringView m_dname;
    KiSStringView m_fname;
    KiSStringView m_type;
    KiSVersion    m_version;
    KiTSize       m_sizeInBytes;
    KiTBool       m_isText;
    KiTUint64     m_createdTs;
    KiTUint64     m_lastEditedTs;
    KiTFlags64    m_flags;
    KiSStringView m_extraMetadata;
    KiSStringView m_fullMetadata;
} KiSStaticResourceMetadata;


/**
 */
KI_INTERFACE(KiIStaticResource) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    /**
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIStaticResource *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     */
    KiTInt32 (KI_CALL *Acquire)(KiIStaticResource *self);
    /**
     */
    KiTInt32 (KI_CALL *Release)(KiIStaticResource *self);

    /**
     */
    KiSStaticResourceMetadata const *(KI_CALL *GetMetadata)(KiIStaticResource *self);
    /**
     */
    KiTVoid const *(KI_CALL *GetPointer)(KiIStaticResource *self);
};

/**
 */
KI_INTERFACE(KiIStaticResourceLocator) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    /**
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIStaticResourceLocator *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     */
    KiTInt32 (KI_CALL *Acquire)(KiIStaticResourceLocator *self);
    /**
     */
    KiTInt32 (KI_CALL *Release)(KiIStaticResourceLocator *self);

    /**
     */
    KiSStaticResourceMetadata const *(KI_CALL *GetStaticResourceMetadataList)(
        KiIStaticResourceLocator *self,
        KiTSize *resMtdCnt
    );
    /**
     */
    KiSStringView const *(KI_CALL *GetPrefix)(KiIStaticResourceLocator *self);

    /**
     */
    KiIStaticResource *(KI_CALL *QueryStaticResource)(KiIStaticResourceLocator *self, KiTChar const *resPath);
};


