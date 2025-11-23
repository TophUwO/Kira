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
 * \file  buffer.c
 * \brief implements the API for the general-purpose buffer object
 */


/* stdlib includes */
#include <stdlib.h>
#include <string.h>

/* Shakai includes */
#include <kira/dbg.h>

#include <kira/common/buffer.h>


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
    return (KiTSize)bufPtr->m_off + reqSpace > bufPtr->m_size;
}

/**
 */
static KiTSize inline KI_CALL KiInternal_BufferComputeNewSize(KiSBuffer const *bufPtr, KiTSize reqSpace) {
    if ((KiTSize)bufPtr->m_off + reqSpace <= bufPtr->m_size)
        return bufPtr->m_size;

    return KI_MAX((KiTSize)bufPtr->m_off + reqSpace, bufPtr->m_size << 1);
}
/** \endcond */


KiSBuffer *KI_CALL KiCreateBuffer(KiTSize initSize) {
    KiSBuffer *bufObj = calloc(1, sizeof *bufObj);
    if (bufObj == nullptr)
        return nullptr;

    /*
     * If initSize is not 0, we also allocate the internal buffer. Otherwise, we simply leave it at zero (guaranteed by
     * calloc()) and allocate it as long as we write data into it.
     */
    if (initSize > 0 && initSize <= (KiTSize)KI_OFFSET_MAX) {
        KiTVoid *intBuf = malloc(initSize);
        if (intBuf == nullptr) {
            free(bufObj);

            return nullptr;
        }

        *bufObj = (KiSBuffer){
            .m_size    = initSize,
            .m_off     = 0,
            .mp_buffer = intBuf
        };
    }

    return bufObj;
}

KiTVoid KI_CALL KiDestroyBuffer(KiSBuffer *bufPtr) {
    if (bufPtr == nullptr)
        return;

    free(bufPtr->mp_buffer);
    free(bufPtr);
}

KiEErrorCode KI_CALL KiReserveBuffer(KiSBuffer *bufPtr, KiTSize s) {
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
    KiTSize const writeCount = bufPtr->m_size / bufSize;
    {
        for (KiTSize i = 0; i < writeCount; i++)
            memcpy((KiTByte *)bufPtr->mp_buffer + i * bufSize, srcBuf, bufSize);
    }
}


KiEErrorCode KI_CALL KiWriteBufferData(KiSBuffer *bufPtr, KiTVoid const *dataPtr, KiTSize s) {
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

    /* All good. */
    bufPtr->m_off += s;
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiReadBufferData(KiSBuffer const *bufPtr, KiTVoid *dstBuf, KiTOffset off, KiTSize s) {
    if (off + s > bufPtr->m_size)
        return KiErr_RangeError;

    memcpy(dstBuf, (KiTByte const *)bufPtr->mp_buffer + off, s);
    return KiErr_Ok;
}

KiTOffset KI_CALL KiSeekBufferPosition(KiSBuffer *bufPtr, KiTOffset off) {
    KiTOffset const oldOff = bufPtr->m_off;
    {
        bufPtr->m_off = KI_MAX(off == KI_SEEK_END ? (KiTOffset)bufPtr->m_size : off, 0);
    }

    return oldOff;
}


KiTOffset KI_CALL KiGetBufferPosition(KiSBuffer const *bufPtr) {
    return bufPtr->m_off;
}

KiTSize KI_CALL KiGetBufferSize(KiSBuffer const *bufPtr) {
    return bufPtr->m_size;
}

KiTVoid const *KI_CALL KiGetBufferPointer(KiSBuffer const *bufPtr, KiTOffset off) {
    if (bufPtr->m_size <= off)
        return nullptr;

    return (KiTByte *)bufPtr->mp_buffer + off;
}


