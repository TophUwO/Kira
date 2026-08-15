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
 * \file  buffer.h
 * \brief defines the API for the general-purpose buffer object
 */


/* Kira includes */
#include <kira/kcm.h>


/**
 */
#define KI_SEEK_BEGIN ((KiTOffset)(0))
/**
 */
#define KI_SEEK_END   ((KiTOffset)(-1))


/** \cond */
KI_NATIVE typedef struct KiSBuffer KiSBuffer;
/** \endcond */


/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiCreateBuffer(KiTSize initSize, KiSBuffer **resPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiCreateBufferFromExisting(KiTSize sizeInBytes, KiTVoid *bufPtr, KiSBuffer **resPtr);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiDestroyBuffer(KiSBuffer *bufPtr);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiReserveBuffer(KiSBuffer *bufPtr, KiTSize s);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiFillBuffer(KiSBuffer *bufPtr, KiTVoid const *srcBuf, KiTSize bufSize);
/**
 */
KI_NATIVE extern KiTVoid KI_CALL KiAttachBuffer(KiSBuffer *bufPtr, KiTVoid const *rawBufPtr, KiTSize sizeInBytes, KiTOffset offset);
/**
 */
KI_NATIVE extern KiTVoid *KI_CALL KiDetachBuffer(KiSBuffer *bufPtr, KiTSize *sizePtr, KiTOffset *offPtr);

/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiWriteBufferData(KiSBuffer *bufPtr, KiTVoid const *dataPtr, KiTSize s);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiWriteStringToBuffer(KiSBuffer *bufPtr, KiTChar const *fmtStr, ...);
/**
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiReadBufferData(KiSBuffer const *bufPtr, KiTVoid *dstBuf, KiTOffset off, KiTSize s);
/**
 */
KI_NATIVE extern KiTOffset KI_CALL KiSeekBufferPosition(KiSBuffer *bufPtr, KiTOffset off);

/**
 */
KI_NATIVE extern KiTOffset KI_CALL KiGetBufferPosition(KiSBuffer const *bufPtr);
/**
 */
KI_NATIVE extern KiTVoid const *KI_CALL KiGetBufferPointer(KiSBuffer const *bufPtr, KiTOffset off);
/**
 */
KI_NATIVE extern KiTSize KI_CALL KiGetBufferSize(KiSBuffer const *bufPtr);
