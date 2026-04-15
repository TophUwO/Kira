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
 * \file  alloc.c
 * \brief implements the minimal KiOM pool allocator
 */


/* stdlib includes */
#include <stdint.h>

#include <string.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/alloc.h>

#include <kira/kernel/int/sync.h>
#include <kira/kernel/int/krnlmod.h>
#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
#define KI_KRNLALLOC_POOLSIZE  ((KiTSize)(16 * 1024 * 1024))
/**
 */
#define KI_KRNLALLOC_POOLCNT   ((KiTSize)(1024 * 1024))
/**
 */
#define KI_KRNLALLOC_MINCOMMIT ((KiTSize)(16))


/**
 * \struct KiSPoolAllocation
 * \brief  represents a raw pool allocation
 */
KI_NATIVE typedef struct KiSPoolAllocation {
    KiTSize  m_nPgTotal;     /**< total number of pages in the pool */
    KiTSize  m_nPgComm;      /**< number of currently committed pages */
    KiTVoid *mp_rawBase;     /**< raw base pointer (possibly unaligned) */
    KiTVoid *mp_alignedBase; /**< pool base pointer (aligned to pool size) */
} KiSPoolAllocation;

/**
 */
KI_NATIVE typedef struct KiSMemoryPool {
    KI_A64 KI_FORCESIZE(64, {
        KiTUint32          m_blockSize;
        KiTUint32          m_blockCap;
        KiTUint32          m_blockCount;
        KiTUint16          m_blockAlign;
        KiSPoolAllocation  m_poolAlloc;
        KiTVoid           *mp_freeList;
    });
} KiSMemoryPool;


/**
 */
KI_NATIVE typedef struct KiSPoolAllocatorState {
    KiSMemoryPool   *mp_poolArray;
    KiTRWLockHandle  m_rwLock;
} KiSPoolAllocatorState;

/**
 */
static KiSPoolAllocatorState gl_PoolAllocator;


/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlCreatePoolAllocation(
    KiTSize sizeInBytes,
    KiTSize nPgComm,
    KiSPoolAllocation *resPtr
) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutParameter)

    /* Get some constants like page size and actual number of bytes we can initially commit. */
    KiTSize const systemPageSize = KiPlatform_VirtualGetPageSize();
    KiTSize const initCommSize   = KI_CLAMP(nPgComm * systemPageSize, 0, sizeInBytes);

    /*
     * Allocate pool with the same alignment as the size. This is important because then we can simply round down to get
     * the pool base from any address inside the pool.
     */
    KiTVoid *rawPtr;
    KiTVoid *poolStart;
    {
        /* Reserve pool. Commit no pages so far. Then calculate pool start. The pool start is aligned to its size. */
        KiEErrorCode errCode = KiPlatform_VirtualReserve(nullptr, sizeInBytes << 1, &rawPtr);
        if (errCode != KiErr_Ok) {
            *resPtr = (KiSPoolAllocation){ 0 };

            return errCode;
        }
        /*
         * Since our maximum misalignment is size - 1, we can move past our next alignment point by adding size - 1 to
         * our raw pointer. Now, we need to subtract the portion we shot over the next alignment point. We do this by
         * clearing the floor(log2(sizeInBytes - 1)) bits. The result is an address that
         *  (1) lies within the allocated pool,
         *  (2) is at least sizeInBytes bytes before the end of the original allocation, and
         *  (3) is definitely aligned to its size.
         */
        poolStart = (KiTVoid *)(((KiTIntptr)rawPtr + sizeInBytes - 1) & ~(sizeInBytes - 1));

        /* As a start, we commit a few pages so that the allocator can start working immediately. */
        errCode = KiPlatform_VirtualCommit(poolStart, initCommSize, KI_DONTCARE(KiTVoid *));
        if (errCode != KiErr_Ok) {
            KiPlatform_VirtualFree(rawPtr, sizeInBytes << 1);

            *resPtr = (KiSPoolAllocation){ 0 };
            return errCode;
        }
    }

    *resPtr = (KiSPoolAllocation){ sizeInBytes / systemPageSize, initCommSize, rawPtr, poolStart };
    return KiErr_Ok;
}

/**
 */
static KiTVoid inline KI_CALL KiInternal_KrnlDestroyPoolAllocation(KiSPoolAllocation *poolAllocPtr, KiTSize poolSizeInBytes) {
    if (poolAllocPtr == nullptr || poolAllocPtr->mp_rawBase == nullptr)
        return;

    /*
     * Simply free the entire base range. We need to uncommit and free separately because we cannot guarantee that
     * KiPlatform_VirtualFree() will uncommit the pages before it releases them. On Windows, for example, though, this
     * is superfluous because VirtualFree() releases committed ranges just as it does uncommitted ones.
     */
    KiTVoid *rawBase = poolAllocPtr->mp_rawBase;
    {
        KiPlatform_VirtualUncommit(rawBase, poolSizeInBytes);
        KiPlatform_VirtualFree(rawBase, poolSizeInBytes);
    }
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlCommitPoolPages(
    KiSPoolAllocation *poolAllocPtr,
    KiTOffset pgOff,
    KiTSize minPgCnt,
    KiTSize maxPgCnt
) {
    KI_ASSERT(poolAllocPtr != nullptr,      KiErr_InOutParameter);
    KI_ASSERT(poolAllocPtr->m_nPgTotal > 0, KiErr_InOutParameter);
    KI_ASSERT(minPgCnt > 0,                 KiErr_SizeParameter);
    KI_ASSERT(minPgCnt <= maxPgCnt,         KiErr_SizeParameter);

    /* Clamp the size so that we don't overcommit. */
    KiTSize pgCnt = KI_CLAMP(maxPgCnt, minPgCnt, poolAllocPtr->m_nPgTotal - pgOff);
    if (pgCnt < minPgCnt)
        return KiErr_MemoryAllocation;

    KiTSize const systemPageSize = KiPlatform_VirtualGetPageSize();
    /* Now, simply commit them. */
    KiTVoid *stPtr = (KiTByte *)poolAllocPtr->mp_alignedBase + pgOff * systemPageSize;
    {
        /* Allocate the range and update allocation metadata. */
        KiEErrorCode errCode = KiPlatform_VirtualCommit(stPtr, pgCnt * systemPageSize, KI_DONTCARE(KiTVoid *));
        if (errCode != KiErr_Ok)
            return errCode;

        poolAllocPtr->m_nPgComm += pgCnt;
    }

    return KiErr_Ok;
}

/**
 */
static KiTVoid inline *KI_CALL KiInternal_KrnlCalculatePoolBase(KiTVoid const *blockPtr, KiTSize poolSizeInBytes) {
    KI_ASSERT(blockPtr != nullptr,              KiErr_InParameter);
    KI_ASSERT(KI_ISPOWEROFTWO(poolSizeInBytes), KiErr_InParameter);

    return (KiTVoid *)((KiTIntptr)blockPtr & ~(poolSizeInBytes - 1));
}

/**
 */
static KiTSize inline KI_CALL KiInternal_KrnlCalcTotalBlockSize(KiTSize blSizeBytes, KiTSize blAlignBytes) {
    return (blSizeBytes + blAlignBytes - 1) & ~(blAlignBytes - 1);
}

/**
 */
static KiTSize inline KI_CALL KiInternal_KrnlCalculatePoolBlockCapacity(
    KiTSize sizeBytes,
    KiTSize blSizeBytes,
    KiTSize blAlignBytes,
    KiTSize mtdPgCnt
) {
    KI_ASSERT(mtdPgCnt >= 1, KiErr_SizeParameter);

    /* Calculate the sizes for block storage and the space occupied by one block. */
    KiTSize const nBytes4Blocks  = sizeBytes - mtdPgCnt * KiPlatform_VirtualGetPageSize();
    KiTSize const nBytesPerBlock = KiInternal_KrnlCalcTotalBlockSize(blSizeBytes, blAlignBytes);

    return nBytes4Blocks / nBytesPerBlock;
}

/**
 */
static KiTVoid inline *KI_CALL KiInternal_KrnlCalcAddressOfFirstBlock(KiSMemoryPool const *poolPtr) {
    KI_ASSERT(poolPtr != nullptr, KiErr_InParameter);

    return (KiTVoid *)((KiTByte *)poolPtr->m_poolAlloc.mp_alignedBase + (1 << 12));
}

/**
 */
static KiTSize inline KI_CALL KiInternal_KrnlCalcNumberOfPagesForBlock(
    KiSMemoryPool const *poolPtr,
    KiTVoid const *blockStartPtr,
    KiTSize totalBlSize
) {
    KI_ASSERT(poolPtr != nullptr,       KiErr_InParameter);
    KI_ASSERT(blockStartPtr != nullptr, KiErr_InParameter);

    KiTOffset const blStartOff = ((KiTIntptr)blockStartPtr + 0           - (KiTIntptr)poolPtr + (1 << 12)) >> 12;
    KiTOffset const blEndOff   = ((KiTIntptr)blockStartPtr + totalBlSize - (KiTIntptr)poolPtr + (1 << 12)) >> 12;

    return (KiTSize)(blEndOff - blStartOff + 1);
}

/**
 */
static KiTVoid inline *KI_CALL KiInternal_KrnlCalcAddressOfNextBlock(KiSMemoryPool const *poolPtr, KiTSize totalBlSize) {
    KI_ASSERT(poolPtr != nullptr, KiErr_InParameter);

    KiTVoid const *fBlockAddr = KiInternal_KrnlCalcAddressOfFirstBlock(poolPtr);
    {
        return (KiTVoid *)((KiTByte *)fBlockAddr + poolPtr->m_blockCount * totalBlSize);
    }
}

/**
 */
static KiTVoid KI_CALL KiInternal_KrnlAddAllToPoolFreeList(KiSMemoryPool *poolPtr, KiTVoid *firstBlockAddr) {
    KI_ASSERT(poolPtr != nullptr, KiErr_InOutParameter);

    KiTVoid const *firstSlot = firstBlockAddr;
    {
        KiTSize const totalBlSize = KiInternal_KrnlCalcTotalBlockSize(poolPtr->m_blockSize, poolPtr->m_blockAlign);

        for (KiTVoid const **slAddr = (KiTVoid const **)firstSlot; KI_TRUE; slAddr = *(KiTVoid **)slAddr) {
            *(KiTVoid const **)slAddr = (KiTVoid *)((KiTByte *)slAddr + totalBlSize);

            /* If we reached the last slot. Put the current head of free list and break. */
            if ((KiTIntptr)slAddr - (KiTIntptr)firstSlot == (poolPtr->m_blockCap - 1) * totalBlSize) {
                *(KiTVoid const **)slAddr = poolPtr->mp_freeList;

                break;
            }
        }
    }

    poolPtr->mp_freeList = (KiTVoid *)firstSlot;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlCreatePool(
    KiTSize sizeBytes,
    KiTSize blSizeBytes,
    KiTSize blAlignBytes,
    KiTVoid **resPtr
) {
    KI_ASSERT(KI_ISPOWEROFTWO(sizeBytes),                       KiErr_SizeParameter);
    KI_ASSERT(sizeBytes >= KiPlatform_VirtualGetPageSize(),     KiErr_SizeParameter);
    KI_ASSERT(sizeBytes % KiPlatform_VirtualGetPageSize() == 0, KiErr_SizeParameter);
    KI_ASSERT(sizeBytes <= SIZE_MAX >> 1,                       KiErr_SizeParameter);
    KI_ASSERT(blSizeBytes < sizeBytes,                          KiErr_SizeParameter);
    KI_ASSERT(KI_ISPOWEROFTWO(blAlignBytes),                    KiErr_SizeParameter);
    KI_ASSERT(blAlignBytes >= _Alignof(KiTByte),                KiErr_SizeParameter);
    KI_ASSERT(sizeBytes % blAlignBytes == 0,                    KiErr_SizeParameter);
    KI_ASSERT(blSizeBytes >= sizeof(KiTVoid *),                 KiErr_SizeParameter);
    KI_ASSERT(blAlignBytes >= _Alignof(KiTVoid *),              KiErr_SizeParameter);
    KI_ASSERT(resPtr != nullptr,                                KiErr_OutptrParameter);

    /* (0) Calculate the number of blocks our pool can hold. If this is 0, it makes no sense to allocate the pool. */
    KiTSize const blockCap = KiInternal_KrnlCalculatePoolBlockCapacity(sizeBytes, blSizeBytes, blAlignBytes, 1);
    if (blockCap == 0) {
        *resPtr = nullptr;

        return KiErr_SizeParameter;
    }

    /* (1) Create pool allocation. */
    KiSPoolAllocation poolAlloc;
    {
        /* Actually allocate the pool array. */
        KiEErrorCode errCode = KiInternal_KrnlCreatePoolAllocation(sizeBytes, KI_KRNLALLOC_MINCOMMIT + 1, &poolAlloc);
        if (errCode != KiErr_Ok) {
            *resPtr = nullptr;

            return errCode;
        }
    }

    /* (2.1) Initialize pool head. */
    *(KiSMemoryPool *)poolAlloc.mp_alignedBase = (KiSMemoryPool const){
        .m_blockSize  = blSizeBytes,
        .m_blockCap   = blockCap,
        .m_blockCount = 0,
        .m_blockAlign = blAlignBytes,
        .m_poolAlloc  = poolAlloc,
        .mp_freeList  = nullptr
    };
    /* (2.2) Initialize free list. */
    KiInternal_KrnlAddAllToPoolFreeList(
        (KiSMemoryPool *)poolAlloc.mp_alignedBase,
        KiInternal_KrnlCalcAddressOfFirstBlock((KiSMemoryPool *)poolAlloc.mp_alignedBase)
    );

    *resPtr = (KiTVoid *)poolAlloc.mp_alignedBase;
    return KiErr_Ok;
}

/**
 */
static KiTVoid inline KI_CALL KiInternal_KrnlDestroyPool(KiSMemoryPool *poolPtr) {
    KI_ASSERT(poolPtr != nullptr,         KiErr_InOutParameter);
    KI_ASSERT(poolPtr->m_blockCount == 0, KiErr_IllegalMemoryPoolState);

    /* Destroy the pool allocation. */
    KiInternal_KrnlDestroyPoolAllocation(&poolPtr->m_poolAlloc, KI_KRNLALLOC_POOLSIZE);
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlAllocateBlock(KiSMemoryPool *poolPtr, KiTVoid **resPtr) {
    KI_ASSERT(poolPtr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(resPtr != nullptr,  KiErr_OutptrParameter);

lbl_ALLOCVIAFREELIST:
    /* First, we check the free list. */
    if (poolPtr->mp_freeList != nullptr) {
        *resPtr = poolPtr->mp_freeList;

        /* Update state and counters. */
        poolPtr->mp_freeList = *(KiTVoid **)poolPtr->mp_freeList;
        ++poolPtr->m_blockCount;

        return KiErr_Ok;
    }
    KiEErrorCode errCode = KiErr_Ok;

    /* If the free list was empty, we must commit more pages. */
    KiTSize const totalBlSize = KiInternal_KrnlCalcTotalBlockSize(poolPtr->m_blockSize, poolPtr->m_blockAlign);
    {
        KiTVoid *block2Alloc = KiInternal_KrnlCalcAddressOfNextBlock(poolPtr, totalBlSize);

        /* Calculate the number of pages we MUST commit to even allocate the block. */
        KiTSize const pg2Comm = KiInternal_KrnlCalcNumberOfPagesForBlock(poolPtr, block2Alloc, totalBlSize);

        /*
         * Commit some pages. This function will only commit as much as possible without overcommitting. Repeat this
         * until either we succeed or we conclude that we cannot allocate any pages.
         */
        for (KiTSize i = KI_KRNLALLOC_MINCOMMIT + 1; i-- > 0;) {
            errCode = KiInternal_KrnlCommitPoolPages(
                &poolPtr->m_poolAlloc,
                poolPtr->m_poolAlloc.m_nPgComm,
                pg2Comm,
                pg2Comm + i
            );

            if (errCode == KiErr_Ok) {
                /* Allocation succeeded. Add to the free list and try allocation again. */
                KiInternal_KrnlAddAllToPoolFreeList(poolPtr, block2Alloc);

                goto lbl_ALLOCVIAFREELIST;
            }
        }
    }

    /* Could not allocate enough memory to be able to allocate the block. */
    *resPtr = nullptr;
    return errCode;
}

/**
 */
static KiTVoid inline KI_CALL KiInternal_KrnlFreeBlock(KiSMemoryPool *poolPtr, KiTVoid *blockPtr) {
    KI_ASSERT(poolPtr != nullptr, KiErr_InOutParameter);
    
    if (blockPtr == nullptr)
        return;

    /* Add to free list. */
    *(KiTVoid **)blockPtr = poolPtr->mp_freeList;
    poolPtr->mp_freeList  = blockPtr;

    /* Update counters. */
    --poolPtr->m_blockCount;
}

/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlCreateAndAddPool(
    KiTSize sizeBytes,
    KiTSize blSizeBytes,
    KiTSize blAlignBytes,
    KiTVoid **resPtr
) {
    KI_ASSERT(resPtr != nullptr, KiErr_OutptrParameter);

    /* Allocate block for the pool. */
    KiTVoid *blockPtr;
    KiEErrorCode errCode = KiInternal_KrnlAllocateBlock(gl_PoolAllocator.mp_poolArray, &blockPtr);
    if (errCode != KiErr_Ok)
        return errCode;

    /* Create pool. */
    KiTVoid *poolPtr;
    errCode = KiInternal_KrnlCreatePool(sizeBytes, blSizeBytes, blAlignBytes, resPtr);
    if (errCode != KiErr_Ok) {
        KiInternal_KrnlFreeBlock(gl_PoolAllocator.mp_poolArray, blockPtr);

        *resPtr = nullptr;
        return errCode;
    }
    *(KiTVoid **)blockPtr = *resPtr;

    return KiErr_Ok;
}


/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(PoolAllocator)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    memset(&gl_PoolAllocator, 0, sizeof gl_PoolAllocator);

    /* Initialize rwlock. */
    KiEErrorCode errCode = KiCreateRWLock(&gl_PoolAllocator.m_rwLock);
    if (errCode != KiErr_Ok)
        return errCode;

    /* Initialize pool array. */
    errCode = KiInternal_KrnlCreatePool(
        KI_KRNLALLOC_POOLSIZE,
        sizeof(KiTVoid *),
        _Alignof(KiTVoid *),
        (KiTVoid **)&gl_PoolAllocator.mp_poolArray
    );
    if (errCode != KiErr_Ok)
        KiDestroyRWLock(&gl_PoolAllocator.m_rwLock);

    return errCode;

    _Static_assert(KI_ISPOWEROFTWO(KI_KRNLALLOC_POOLSIZE), "Pool-allocator per-pool size must be a power of two.");
    _Static_assert(sizeof(KiSMemoryPool) <= (1 << 12),     "Pool-allocator per-pool metadata must fit into one page.");
}

/**
 */
static KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(PoolAllocator)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /*
     * Destroy all pools that are still there. Since this is a kernel module and no threads should be running when
     * kernel modules are shutdown, there should be no contention here. If there is, then you (i.e., the user of Kira)
     * fucked up big time. Only start threads using Kira's APIs. Do not spawn detached native threads that call the
     * allocator either directly or indirectly.
     */
    KiTByte *currPool = KiInternal_KrnlCalcAddressOfFirstBlock(gl_PoolAllocator.mp_poolArray);
    {
        /* Get the "root" pool. */
        KiSMemoryPool const *const rPool = gl_PoolAllocator.mp_poolArray;

        for (KiTSize i = 0; i < gl_PoolAllocator.mp_poolArray->m_blockCap; i++) {
            if (currPool != nullptr)
                KiInternal_KrnlDestroyPool((KiSMemoryPool *)currPool);

            /* Get next pool. */
            currPool += KiInternal_KrnlCalcTotalBlockSize(rPool->m_blockSize, rPool->m_blockAlign);
        }
    }
    /* Now, delete the "root". */
    KiInternal_KrnlDestroyPool(gl_PoolAllocator.mp_poolArray);

    KiDestroyRWLock(&gl_PoolAllocator.m_rwLock);
    return KiErr_Ok;
}
/** \endcond */


KiEErrorCode KI_CALL KiKrnlAllocateBlock(KiTSize sizeBytes, KiTSize alignBytes, KiTVoid **resPtr) {
    KI_ASSERT(sizeBytes >= sizeof(KiTVoid *),    KiErr_SizeParameter);
    KI_ASSERT(KI_ISPOWEROFTWO(alignBytes),       KiErr_SizeParameter);
    KI_ASSERT(alignBytes >= _Alignof(KiTVoid *), KiErr_SizeParameter);
    KI_ASSERT(resPtr != nullptr,                 KiErr_OutptrParameter);

    KiAcquireWrite(&gl_PoolAllocator.m_rwLock);
    {
        KiEErrorCode errCode = KiErr_Ok;

        /* Locate a suitable memory pool. */
        KiSMemoryPool *foundPool = nullptr;
        KiSMemoryPool *rPool     = gl_PoolAllocator.mp_poolArray;
        KiSMemoryPool *firstPool = KiInternal_KrnlCalcAddressOfFirstBlock(rPool);
        KiTSize        poolAdv   = KiInternal_KrnlCalcTotalBlockSize(rPool->m_blockSize, rPool->m_blockAlign);

        for (KiSMemoryPool *memPool = firstPool; KI_TRUE; memPool = (KiTVoid *)((KiTByte *)memPool + poolAdv)) {
            /* Check if the pool has the properties we want. */
            if (memPool != nullptr && memPool->m_blockSize == sizeBytes && memPool->m_blockAlign == alignBytes) {
                /* Found a pool candidate. Now we must check if the pool has free space. */
                if (memPool->mp_freeList != nullptr || memPool->m_blockCount < memPool->m_blockCap) {
                    /* We got free space. Choose this pool. */
                    foundPool = memPool;

                    goto lbl_ALLOCATEBLOCK;
                }
            }

            /* Check if we reached the end of the current array. */
            if ((KiTIntptr)memPool - (KiTIntptr)firstPool == (rPool->m_blockCap - 1) * poolAdv)
                break;
        }
        /* If we could not find a pool, we must allocate one. */
        errCode = KiInternal_KrnlCreateAndAddPool(KI_KRNLALLOC_POOLSIZE, sizeBytes, alignBytes, (KiTVoid **)&foundPool);
        if (errCode != KiErr_Ok) {
            KiReleaseWrite(&gl_PoolAllocator.m_rwLock);

            *resPtr = nullptr;
            return errCode;
        }

lbl_ALLOCATEBLOCK:
        /* Now we have a pool we can use. Allocate a block inside there. */
        errCode = KiInternal_KrnlAllocateBlock(foundPool, resPtr);
        if (errCode != KiErr_Ok) {
            KiReleaseWrite(&gl_PoolAllocator.m_rwLock);

            *resPtr = nullptr;
            return errCode;
        }
    }
    KiReleaseWrite(&gl_PoolAllocator.m_rwLock);

    /* All good. We have successfully reserved block we can use. */
    return KiErr_Ok;
}

KiTVoid KI_CALL KiKrnlFreeBlock(KiTVoid *blockPtr) {
    if (blockPtr == nullptr)
        return;

    KiAcquireWrite(&gl_PoolAllocator.m_rwLock);
    {
        /*
         * Get pool from block. Because the pool size is aligned to its size and the size is constant, we can simply
         * truncate the block address to get the base address.
         */
        KiSMemoryPool *poolPtr = KiInternal_KrnlCalculatePoolBase(blockPtr, KI_KRNLALLOC_POOLSIZE);
        {
            KiInternal_KrnlFreeBlock(poolPtr, blockPtr);
        }
    }
    KiReleaseWrite(&gl_PoolAllocator.m_rwLock);
}

KiTSize KI_CALL KiKrnlGetBlockSize(KiTVoid const *blockPtr) {
    KI_ASSERT(blockPtr != nullptr, KiErr_InParameter);

    KiTSize res;
    KiAcquireRead(&gl_PoolAllocator.m_rwLock);
    {
        KiSMemoryPool const *memPool = KiInternal_KrnlCalculatePoolBase(blockPtr, KI_KRNLALLOC_POOLSIZE);

        res = memPool->m_blockSize;
    }
    KiReleaseRead(&gl_PoolAllocator.m_rwLock);

    return res;
}


/** \cond */
KI_KRNLMOD(PoolAllocator,
    KI_EXPAND({
        KI_KRNLMOD_DEPENDENCY(0xFFFFFFFF, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC),
        KI_KRNLMOD_DEPENDENCY(0xFFFFFFFF, 0xAAAAAAAA, 0xBBBBBBBB, 0xCCCCCCCC)
    }), 
{
    .mp_modUuid  = &KI_MAKE_UUID(0, 0, 0, 0),
    .mp_modId    = &KI_MAKE_STRING_VIEW("pool allocator"),
    .m_modFlags  = 0
});
/** \endcond */


