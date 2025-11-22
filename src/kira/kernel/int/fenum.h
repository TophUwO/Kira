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
 * \file  enum.h
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
    KiSKrnlString *mp_fullPath;
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
KI_NATIVE extern KiSFileEnumerationContext *KI_CALL KiFileEnumerationContextCreate(
    KiSFileEnumerationProperties const *enumPropsPtr
);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiFileEnumerationContextDestroy(KiSFileEnumerationContext *ctxtPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiFileEnumerationContextReset(KiSFileEnumerationContext *ctxtPtr);
/**
 */
KI_NATIVE extern KiSFileEnumerationResult *KI_CALL KiFileEnumerationContextYield(KiSFileEnumerationContext *ctxtPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiFileEnumerationContextDiscard(KiSFileEnumerationContext *ctxtPtr);
/**
 */
KI_NATIVE extern KiSFileEnumerationProperties const *KI_CALL KiFileEnumerationContextGetProperties(
    KiSFileEnumerationContext const *ctxtPtr
);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiSFileEnumerationContextGetLastError(KiSFileEnumerationContext const *ctxtPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiFileEnumerationContextDestroyResult(KiSFileEnumerationResult *resPtr);


