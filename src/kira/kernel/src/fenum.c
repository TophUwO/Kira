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
 * \file  fenum.c
 * \brief implements the kernel-level file enumeration system
 */


/* stdlib includes */
#include <stdlib.h>

#include <errno.h>

/* external/stdlib includes */
#if (defined KI_PLATFORM_WINDOWS)
    #include <kira/kernel/ext/dirent/dirent.h>
#else
    #include <dirent.h>
#endif

/* Kira includes */
#include <kira/kernel/reg.h>
#include <kira/kernel/dir.h>

#include <kira/kernel/int/fenum.h>
#include <kira/kernel/int/string.h>
#include <kira/kernel/int/gparray.h>

#include <kira/dbg/dbg.h>


/** \cond INTERNAL */
struct KiSKrnlFileEnumerationContext {
    KiEErrorCode                      m_lastError;
    KiTBool                           m_isError;
    KiTBool                           m_isInit;
    KiSKrnlFileEnumerationResult     *mp_currResPtr;
    KiSKrnlString                    *mp_currElem;
    KiSKrnlGPArray                   *mp_dirStack;
    DIR                              *mp_currDirHnd;
    KiTInt32                          m_currResIndex;
    KiTInt32                          m_nResProc;
    KiSKrnlFileEnumerationProperties  m_enumProps;
};


/**
 */
static KiEErrorCode KI_CALL KiInternal_KrnlFileEnumerationContextReset(
    KiSKrnlFileEnumerationContext *ctxtPtr,
    KiSKrnlFileEnumerationProperties const *enumPropsPtr
) {
    KI_ASSERT(enumPropsPtr != nullptr, KiErr_InParameter);

    /* Copy root directory. */
    KiSKrnlString *currDirPath;
    KiEErrorCode errCode = KiKrnlStringCreate(enumPropsPtr->mp_rootDir, &currDirPath);
    if (errCode != KiErr_Ok) {
        free(ctxtPtr);

        return errCode;
    }
    /* Create directory stack. */
    KiSKrnlGPArray *dirStack;
    errCode = KiKrnlGPArrayCreate(&dirStack);
    if (errCode != KiErr_Ok) {
        KiKrnlStringDestroy(currDirPath);

        return errCode;
    }
    /* Open root directory. */
    DIR *currDir = opendir(enumPropsPtr->mp_rootDir);
    if (currDir == nullptr) {
        KiKrnlGPArrayDestroy(dirStack);
        KiKrnlStringDestroy(currDirPath);

        return (errCode = KiKrnlErrnoToKiraErrorCode(errno)) != KiErr_Unknown
            ? errCode
            : KiErr_MemoryAllocation
        ;
    }

    /* Finally, initialize context. */
    *ctxtPtr = (KiSKrnlFileEnumerationContext){
        .m_lastError    = KiErr_Ok,
        .m_isError      = KI_FALSE,
        .m_isInit       = KI_TRUE,
        .m_currResIndex = -1,
        .m_nResProc     = 0,
        .mp_currResPtr  = nullptr,
        .mp_dirStack    = dirStack,
        .mp_currElem    = currDirPath,
        .mp_currDirHnd  = currDir,
        .m_enumProps    = *enumPropsPtr
    };
    return KiErr_Ok;
}

/**
 */
static KiTVoid KI_CALL KiInternal_KrnlFileEnumerationContextUninit(KiSKrnlFileEnumerationContext *ctxtPtr) {
    KI_ASSERT(ctxtPtr != nullptr, KiErr_InOutParameter);

    /* Destroy the current directory and the directory stack. */
    KiTSize nElem;
    KiTVoid **elemPtr = KiKrnlGPArrayMap(ctxtPtr->mp_dirStack, 0, -1, &elemPtr, &nElem);
    {
        if (elemPtr == nullptr)
            goto lbl_DESTROYSTRUCT;

        /*
         * Destroy all elements. Then destroy the stack itself. This is because the stack must be empty when it is
         * destroyed.
         */
        for (KiTSize i = 0; i < nElem; i++) {
            if (elemPtr[i] == nullptr)
                continue;

            closedir((DIR *)elemPtr[i]);
        }
        KiKrnlGPArrayDestroy(ctxtPtr->mp_dirStack);
    }
    
    /* Destroy the rest of the structure. */
lbl_DESTROYSTRUCT:
    closedir(ctxtPtr->mp_currDirHnd);

    KiKrnlStringDestroy(ctxtPtr->mp_currElem);
}

/**
 */
static KiSKrnlFileEnumerationResult *KI_CALL KiInternal_KrnlFileEnumerationContextCreateResult(
    KiSKrnlFileEnumerationContext const *ctxtPtr,
    KiEErrorCode *resErrCode
) {
    KI_ASSERT(ctxtPtr != nullptr,    KiErr_InParameter);
    KI_ASSERT(resErrCode != nullptr, KiErr_OutParameter);

    /* Allocate module result. */
    KiSKrnlFileEnumerationResult *newRes = malloc(sizeof *newRes);
    if (newRes != nullptr) {
        *resErrCode = KiErr_MemoryAllocation;

        return nullptr;
    }

    KiSKrnlString *fullPath;
    {
        /* Duplicate the full path. */
        KiTChar      const *fullCPath = KiKrnlStringCStr(ctxtPtr->mp_currElem);
        KiEErrorCode        errCode   = KiKrnlStringDuplicate(ctxtPtr->mp_currElem, &fullPath);
        if (errCode != KiErr_Ok) {
            free(newRes);

            *resErrCode = KiErr_MemoryAllocation;
            return nullptr;
        }
        /* Construct. */
        *newRes = (KiSKrnlFileEnumerationResult){
            .m_structSize    = sizeof *newRes,
            .m_resultIndex   = ctxtPtr->m_currResIndex,
            .mp_fullPath     = fullPath,
            .m_dirPath       = KI_MAKE_STRING_VIEW(""),
            .m_fileDirName   = KI_MAKE_STRING_VIEW(""),
            .m_fileName      = KI_MAKE_STRING_VIEW(""),
            .m_fileExt       = KI_MAKE_STRING_VIEW("")
        };
        KiKrnlSplitPath(fullCPath, &newRes->m_dirPath, &newRes->m_fileDirName, &newRes->m_fileName, &newRes->m_fileExt);
    }

    /* All good. */
    *resErrCode = KiErr_Ok;
    return newRes;
}

/**
 */
static KiSKrnlFileEnumerationResult KI_CALL *KiInternal_KrnlFileEnumerationContextYieldOne(
    KiSKrnlFileEnumerationContext *ctxtPtr
) {
    KI_ASSERT(ctxtPtr != nullptr, KiErr_InOutParameter);

    /* Current directory is nullptr. This means we are done iterating. */
    if (ctxtPtr->mp_currDirHnd == nullptr)
        return nullptr;

    /* Read from the current directory. */
    KiEErrorCode errCode = KiErr_Ok;
    do {
        struct dirent *currRes = nullptr;

        if ((currRes = readdir(ctxtPtr->mp_currDirHnd)) != nullptr) {
            /* Push new component to stack. */
            KiKrnlStringPushPathComponent(ctxtPtr->mp_currElem, currRes->d_name);

            switch (currRes->d_type) {
                case DT_DIR:
                    /* Found a directory. Open it and set it as current. */
                    {
                        /* Kiip '.' and '..' "directories". */
                        if (!strcmp(currRes->d_name, ".") || !strcmp(currRes->d_name, "..")) {
                            KiKrnlStringPopPathComponent(ctxtPtr->mp_currElem);

                            continue;
                        }

                        /* Push current directory. Save it for later. */
                        KiKrnlGPArrayPush(ctxtPtr->mp_dirStack, ctxtPtr->mp_currDirHnd);
                        DIR *newDir = opendir(KiKrnlStringCStr(ctxtPtr->mp_currElem));
                        {
                            if (newDir == nullptr) {
                                ctxtPtr->mp_currDirHnd = KiKrnlGPArrayPop(ctxtPtr->mp_dirStack);

                                errCode = KiKrnlErrnoToKiraErrorCode(errno);
                                goto lbl_ONERROR;
                            }

                            /* Enter directory and try to yield a result from there. */
                            continue;
                        }
                    }

                    break;
                case DT_REG:
                    /* Get the result. */
                    KiSKrnlFileEnumerationResult *newRes = KiInternal_KrnlFileEnumerationContextCreateResult(
                        ctxtPtr,
                        &errCode
                    );
                
                    /*
                     * Yield module. Before we return, we pop the entity name because when we return, it should always
                     * contain a directory path.
                     */
                    KiKrnlStringPopPathComponent(ctxtPtr->mp_currElem);
                    if (newRes == nullptr)
                        goto lbl_ONERROR;
                    return newRes;
                case DT_LNK:
                    if (!ctxtPtr->m_enumProps.m_doWalkSymlinks)
                        break;

                    /* Handle the symlink like a normal directory if it points to a directory. */
                    /// TODO: implement
                default:
                    /*
                     * If it's some other weird abomination of filesystem entity, we simply revert the entity name push.
                     */
                    KiKrnlStringPopPathComponent(ctxtPtr->mp_currElem);
            }
        } else {
            /*
             * Could not read directory. The current directory contains no more files. Restore the old state and try to
             * fetch another one.
             */
            closedir(ctxtPtr->mp_currDirHnd);
            KiKrnlStringPopPathComponent(ctxtPtr->mp_currElem);
            ctxtPtr->mp_currDirHnd = KiKrnlGPArrayPop(ctxtPtr->mp_dirStack);

            continue;
        }
    } while (KI_TRUE);

    /* Could not yield result. Set error code and return nullptr. */
lbl_ONERROR:
    ctxtPtr->m_isError   = KI_TRUE;
    ctxtPtr->m_lastError = errCode;

    return nullptr;
}
/** \endcond */


KiSKrnlFileEnumerationContext *KI_CALL KiKrnlFileEnumerationContextCreate(
    KiSKrnlFileEnumerationProperties const *enumPropsPtr
) {
    KI_ASSERT(enumPropsPtr != nullptr, KiErr_InParameter);

    /* Allocate memory for the context. */
    KiSKrnlFileEnumerationContext *ctxtPtr = malloc(sizeof *ctxtPtr);
    if (ctxtPtr == nullptr)
        return nullptr;
        
    /* Prepare context for first use. */
    KiEErrorCode errCode = KiInternal_KrnlFileEnumerationContextReset(ctxtPtr, enumPropsPtr);
    if (errCode != KiErr_Ok) {
        free(ctxtPtr);

        return nullptr;
    }
    /* All good. */
    return ctxtPtr;
}

KiTVoid KI_CALL KiKrnlFileEnumerationContextDestroy(KiSKrnlFileEnumerationContext *ctxtPtr) {
    if (ctxtPtr == nullptr)
        return;

    /* Uninit context. */
    KiInternal_KrnlFileEnumerationContextUninit(ctxtPtr);

    /* Free structure. */
    free(ctxtPtr);
}

KiTVoid KI_CALL KiKrnlFileEnumerationContextReset(KiSKrnlFileEnumerationContext *ctxtPtr) {
    KI_ASSERT(ctxtPtr != nullptr, KiErr_InOutParameter);

    KiEErrorCode errCode;
    auto oldProps = ctxtPtr->m_enumProps;
    {
        /* Uninitialize. This destroys the old state. */
        KiInternal_KrnlFileEnumerationContextUninit(ctxtPtr);

        /* Initialize. This simply creates a new fresh state. */
        errCode = KiInternal_KrnlFileEnumerationContextReset(ctxtPtr, &oldProps);
    }

    /* If an error happened, set the flag. */
    ctxtPtr->m_isError   = errCode != KiErr_Ok;
    ctxtPtr->m_lastError = errCode != KiErr_Ok ? errCode : ctxtPtr->m_lastError;
}

KiSKrnlFileEnumerationResult *KI_CALL KiKrnlFileEnumerationContextYield(KiSKrnlFileEnumerationContext *ctxtPtr) {
    KI_ASSERT(ctxtPtr != nullptr, KiErr_InOutParameter);

    /*
     * Preparation stage.
     *  (1) Check if error flag is set. If yes, do nothing and return nullptr.
     *  (2) Update the indices.
     */
    {
        /* Simply do nothing if the error flag is set. */
        if (ctxtPtr->m_isError)
            return nullptr;

        /* Update indices. */
        ++ctxtPtr->m_currResIndex;
        ++ctxtPtr->m_nResProc;
    }

    /* Yield one result. */
    return KiInternal_KrnlFileEnumerationContextYieldOne(ctxtPtr);
}

KiTVoid KI_CALL KiKrnlFileEnumerationContextDiscard(KiSKrnlFileEnumerationContext *ctxtPtr) {
    KI_ASSERT(ctxtPtr != nullptr, KiErr_InOutParameter);

    /* Do nothing if error flag is set. */
    if (ctxtPtr->m_isError)
        return;

    KiKrnlFileEnumerationContextDestroyResult(ctxtPtr->mp_currResPtr);
}

KiSKrnlFileEnumerationProperties const *KI_CALL KiKrnlFileEnumerationContextGetProperties(
    KiSKrnlFileEnumerationContext const *ctxtPtr
) {
    KI_ASSERT(ctxtPtr != nullptr, KiErr_InParameter);

    return (KiSKrnlFileEnumerationProperties const *)&ctxtPtr->m_enumProps;
}

KiEErrorCode KI_CALL KiSKrnlFileEnumerationContextGetLastError(KiSKrnlFileEnumerationContext const *ctxtPtr) {
    KI_ASSERT(ctxtPtr != nullptr, KiErr_InParameter);

    return ctxtPtr->m_lastError;
}

KiTVoid KI_CALL KiKrnlFileEnumerationContextDestroyResult(KiSKrnlFileEnumerationResult *resPtr) {
    if (resPtr == nullptr)
        return;

    /* Only actually free the full path as the separated components are simply pointers into the full path string. */
    KiKrnlStringDestroy(resPtr->mp_fullPath);

    /* Destroy container structure. */
    free(resPtr);
}


