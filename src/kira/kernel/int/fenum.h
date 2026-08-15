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
 * \file  fenum.h
 * \brief defines the API for the kernel-level file enumeration system
 */


#pragma once

/* Kira includes */
#include <kira/kernel/error.h>

#include <kira/kernel/int/string.h>


/**
 */
KI_NATIVE typedef struct KiSFileEnumerationContext KiSFileEnumerationContext;


/**
 */
KI_NATIVE typedef struct KiSFileEnumerationResult {
    KiTSize        m_structSize;
    KiTInt32       m_resultIndex;
    KiSString     *mp_fullPath;
    KiSStringView  m_dirPath;
    KiSStringView  m_fileDirName;
    KiSStringView  m_fileName;
    KiSStringView  m_fileExt;
} KiSFileEnumerationResult;

/**
 */
KI_NATIVE typedef struct KiSFileEnumerationProperties {
    KiTSize         m_structSize;
    KiTInt32        m_nSkipRes;
    KiTInt32        m_nMaxCount;
    KiTBool         m_isRecursive;
    KiTBool         m_doWalkSymlinks;
    KiTChar  const *mp_rootDir;
} KiSFileEnumerationProperties;


/**
 */
KI_NATIVE extern KiSFileEnumerationContext *KI_CALL KiCreateFEC(KiSFileEnumerationProperties const *enumPropsPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiDestroyFEC(KiSFileEnumerationContext *ctxtPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiResetFEC(KiSFileEnumerationContext *ctxtPtr);
/**
 */
KI_NATIVE extern KiSFileEnumerationResult *KI_CALL KiYieldFromFEC(KiSFileEnumerationContext *ctxtPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiDiscardFECResult(KiSFileEnumerationContext *ctxtPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiDestroyFECResult(KiSFileEnumerationResult *resPtr);

/**
 */
KI_NATIVE extern KiSFileEnumerationProperties const *KI_CALL KiGetFECProperties(KiSFileEnumerationContext const *ctxtPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiGetFECLastError(KiSFileEnumerationContext const *ctxtPtr);
