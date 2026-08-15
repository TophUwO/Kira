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
 * \file  buffer.c
 * \brief implements the API for the general-purpose buffer object
 */


/* stdlib includes */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <string.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/int/buffer.h>


/** \cond INTERNAL */
/**
 */
struct KiSBuffer {
    KiTSize    m_size;
    KiTOffset  m_off;
    KiTVoid   *mp_buffer;
};


/**
 */
static KiTBool inline KI_CALL KiInternal_BufferNeedsResize(KiSBuffer const *bufPtr, KiTSize reqSpace) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InParameter);

    return (KiTSize)bufPtr->m_off + reqSpace > bufPtr->m_size;
}

/**
 */
static KiTSize inline KI_CALL KiInternal_BufferComputeNewSize(KiSBuffer const *bufPtr, KiTSize reqSpace) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InParameter);

    if ((KiTSize)bufPtr->m_off + reqSpace <= bufPtr->m_size)
        return bufPtr->m_size;

    return KI_MAX((KiTSize)bufPtr->m_off + reqSpace, bufPtr->m_size << 1);
}
/** \endcond */


KiEErrorCode KI_CALL KiCreateBuffer(KiTSize initSize, KiSBuffer **resPtr) {
    if ((*resPtr = calloc(1, sizeof **resPtr)) == nullptr)
        return KiErr_MemoryAllocation;

    /*
     * If initSize is not 0, we also allocate the internal buffer. Otherwise, we simply leave it at zero (guaranteed by
     * calloc()) and allocate it as long as we write data into it.
     */
    if (initSize > 0 && initSize <= (KiTSize)KI_OFFSET_MAX) {
        KiTVoid *intBuf = malloc(initSize);
        if (intBuf == nullptr) {
            free(*resPtr);

            *resPtr = nullptr;
            return KiErr_MemoryAllocation;
        }

        **resPtr = (KiSBuffer){
            .m_size    = initSize,
            .m_off     = 0,
            .mp_buffer = intBuf
        };
    }

    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiCreateBufferFromExisting(KiTSize sizeInBytes, KiTVoid *bufPtr, KiSBuffer **resPtr) {
    KI_ASSERT(sizeInBytes > 0,   KiErr_InParameter);
    KI_ASSERT(bufPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    if ((*resPtr = malloc(sizeof **resPtr)) == nullptr)
        return KiErr_MemoryAllocation;

    **resPtr = (KiSBuffer){
        .m_size    = sizeInBytes,
        .m_off     = sizeInBytes - 1,
        .mp_buffer = bufPtr,
    };
    return KiErr_Ok;
}

KiTVoid KI_CALL KiDestroyBuffer(KiSBuffer *bufPtr) {
    if (bufPtr == nullptr)
        return;

    free(bufPtr->mp_buffer);
    free(bufPtr);
}

KiEErrorCode KI_CALL KiReserveBuffer(KiSBuffer *bufPtr, KiTSize s) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InOutParameter);

    if (s == 0)
        return KiErr_Ok;

    KiTSize const newSize = bufPtr->m_size + s;
    {
        KiTVoid *newBuf = realloc(bufPtr->mp_buffer, newSize);
        if (newBuf == nullptr)
            return KiErr_MemoryReallocation;

        bufPtr->mp_buffer = newBuf;
        bufPtr->m_size    = newSize;
    }

    return KiErr_Ok;
}

KiTVoid KI_CALL KiFillBuffer(KiSBuffer *bufPtr, KiTVoid const *srcBuf, KiTSize bufSize) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(srcBuf != nullptr, KiErr_InParameter);
    KI_ASSERT(bufSize > 0,       KiErr_InParameter);

    KiTSize const writeCount = bufPtr->m_size / bufSize;
    {
        for (KiTSize i = 0; i < writeCount; i++)
            memcpy((KiTByte *)bufPtr->mp_buffer + i * bufSize, srcBuf, bufSize);
    }

    /* Write the remainder. */
    if (bufPtr->m_size % bufSize != 0)
        memcpy(
            (KiTByte *)bufPtr->mp_buffer + writeCount * bufSize,
            srcBuf,
            bufPtr->m_size % bufSize
        );
}

KiTVoid KI_CALL KiAttachBuffer(KiSBuffer *bufPtr, KiTVoid const *rawBufPtr, KiTSize sizeInBytes, KiTOffset offset) {
    KI_ASSERT(bufPtr != nullptr,    KiErr_InOutParameter);
    KI_ASSERT(rawBufPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(sizeInBytes > 0,      KiErr_InParameter);
    KI_ASSERT(offset >= 0,          KiErr_InParameter);

    if (bufPtr->mp_buffer != nullptr) {
        /* Buffer already attached. */
        return;
    }

    *bufPtr = (KiSBuffer){
        .m_size    = sizeInBytes,
        .m_off     = offset,
        .mp_buffer = (KiTVoid *)rawBufPtr
    };
}

KiTVoid *KI_CALL KiDetachBuffer(KiSBuffer *bufPtr, KiTSize *sizePtr, KiTOffset *offPtr) {
    KI_ASSERT(bufPtr != nullptr,  KiErr_InOutParameter);
    KI_ASSERT(sizePtr != nullptr, KiErr_OutParameter);
    KI_ASSERT(offPtr != nullptr,  KiErr_OutParameter);

    if (bufPtr->mp_buffer == nullptr) {
        /* No buffer attached. */
        *sizePtr = 0;
        *offPtr  = -1;

        return nullptr;
    }
    KiTVoid *resPtr = bufPtr->mp_buffer;
    *sizePtr = bufPtr->m_size;
    *offPtr  = bufPtr->m_off;

    *bufPtr = (KiSBuffer){
        .m_size    = 0,
        .m_off     = -1,
        .mp_buffer = nullptr
    };
    return resPtr;
}


KiEErrorCode KI_CALL KiWriteBufferData(KiSBuffer *bufPtr, KiTVoid const *dataPtr, KiTSize s) {
    KI_ASSERT(bufPtr != nullptr,  KiErr_InOutParameter);
    KI_ASSERT(dataPtr != nullptr, KiErr_InParameter);

    if (s == 0)
        return KiErr_Ok;
    
    if (KiInternal_BufferNeedsResize(bufPtr, s)) {
        /* Compute new buffer size. */
        KiTSize const newSize = KiInternal_BufferComputeNewSize(bufPtr, s);
        if (newSize > (KiTSize)KI_OFFSET_MAX)
            return KiErr_ContainerFull;

        /* Enlarge buffer. If this fails, the current buffer is not touched. */
        KiTVoid *newBuf = realloc(bufPtr->mp_buffer, newSize);
        {
            if (newBuf == nullptr)
                return KiErr_MemoryReallocation;

            bufPtr->mp_buffer = newBuf;
            bufPtr->m_size    = newSize;
        }
    }
    memcpy((KiTByte *)bufPtr->mp_buffer + bufPtr->m_off, dataPtr, s);

    bufPtr->m_off += s;
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiWriteStringToBuffer(KiSBuffer *bufPtr, KiTChar const *fmtStr, ...) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(fmtStr != nullptr, KiErr_InParameter);

    /* Make space for the string if needed. */
    KiTInt32 reqSize = 0;

    va_list vlArgs;
    va_start(vlArgs, fmtStr);
    {
        reqSize = vsnprintf(nullptr, 0, fmtStr, vlArgs);
        if (reqSize < 0) {
            va_end(vlArgs);
            
            return KiErr_EncodingError;
        }
        ++reqSize;

        KiEErrorCode errCode = KiReserveBuffer(bufPtr, KI_MAX(0, bufPtr->m_off + reqSize - (KiTOffset)bufPtr->m_size));
        if (errCode != KiErr_Ok) {
            va_end(vlArgs);

            return errCode;
        }
    }
    va_end(vlArgs);

    /* Write the string. We will have enough space in the buffer. */
    va_start(vlArgs, fmtStr);
    {
        vsnprintf((KiTChar *)bufPtr->mp_buffer + bufPtr->m_off, reqSize, fmtStr, vlArgs);

        /* Write NUL-terminator and update offset. */
        ((KiTChar *)bufPtr->mp_buffer)[bufPtr->m_off + reqSize - 1] = '\0';
        KiSeekBufferPosition(bufPtr, bufPtr->m_off + reqSize);
    }
    va_end(vlArgs);

    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiReadBufferData(KiSBuffer const *bufPtr, KiTVoid *dstBuf, KiTOffset off, KiTSize s) {
    KI_ASSERT(bufPtr != nullptr,         KiErr_InParameter);
    KI_ASSERT(dstBuf != nullptr,         KiErr_OutParameter);
    KI_ASSERT(off + s <= bufPtr->m_size, KiErr_RangeError);

    memcpy(dstBuf, (KiTByte const *)bufPtr->mp_buffer + off, s);
    return KiErr_Ok;
}

KiTOffset KI_CALL KiSeekBufferPosition(KiSBuffer *bufPtr, KiTOffset off) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InOutParameter);

    KiTOffset const oldOff = bufPtr->m_off;
    {
        bufPtr->m_off = KI_MAX(off == KI_SEEK_END ? (KiTOffset)bufPtr->m_size : off, 0);
    }

    return oldOff;
}


KiTOffset KI_CALL KiGetBufferPosition(KiSBuffer const *bufPtr) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InParameter);

    return bufPtr->m_off;
}

KiTSize KI_CALL KiGetBufferSize(KiSBuffer const *bufPtr) {
    KI_ASSERT(bufPtr != nullptr, KiErr_InParameter);

    return bufPtr->m_size;
}

KiTVoid const *KI_CALL KiGetBufferPointer(KiSBuffer const *bufPtr, KiTOffset off) {
    KI_ASSERT(bufPtr != nullptr,    KiErr_InParameter);
    KI_ASSERT(bufPtr->m_size > off, KiErr_InParameter);

    return (KiTByte *)bufPtr->mp_buffer + off;
}


