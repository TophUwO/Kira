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
 * \file  linfile.c
 * \brief implements Linux-specific basic file handling routines
 */
#if (defined KI_PLATFORM_LINUX)


/* stdlib includes */
#include <stdio.h>
#include <errno.h>

/* Linux includes */
#include <unistd.h>

#include <sys/stat.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/error.h>

#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 * 
 */
static KiEErrorCode KI_CALL KiInternal_KiraFileAccessModeToFopenFileAccessMode(KiEFileAccessMode mode, char *resPtr) {
    KiTOffset currOff = 0;

    /* Stage 1: General access mode. */
    switch ((KiTUint32)mode & 0x000000FF) {
        case KiFAccMd_Read:   resPtr[currOff] = 'r'; break;
        case KiFAccMd_Write:  resPtr[currOff] = 'w'; break;
        case KiFAccMd_Append: resPtr[currOff] = 'a'; break;
        default:
            return KiErr_InvalidFileAccessMode;
    }
    ++currOff;

    /* Stage 2: Update mode. */
    switch ((KiTUint32)mode & 0x0000FF00) {
        case KiFAccMd_Update:
            resPtr[currOff] = '+';

            ++currOff;
            break;
        case KiFAccMd_None: break;
        default:
            return KiErr_InvalidFileAccessMode;
    }

    /* Stage 3: Content mode. */
    switch ((KiTUint32)mode & 0x00FF0000) {
        case KiFAccMd_Binary:
            resPtr[currOff] = 'b';
            
            ++currOff;
            break;
        case KiFAccMd_None:
        case KiFAccMd_Text: break;
        default:
            return KiErr_InvalidFileAccessMode;
    }

    /* Stage 4: C11 extended access mode. */
    switch ((KiTUint32)mode & 0xFF000000) {
        case KiFAccMd_MustNotExist:
            if (((KiTUint32)mode & KiFAccMd_Write) != 0) {
                resPtr[currOff] = 'x';

                break;
            }

            return KiErr_InvalidFileAccessMode;
        case KiFAccMd_MustExist: break;
        default:
            return KiErr_InvalidFileAccessMode;
    }
    ++currOff;

    resPtr[currOff] = '\0';
    return KiErr_Ok;
}

/**
 *
 */
static int KI_CALL KiInternal_KiraFPOriginToFseekOrigin(KiEFilePositionOrigin origin) {
    switch (origin) {
        case KiFPOri_Set: return SEEK_SET;
        case KiFPOri_Cur: return SEEK_CUR;
        case KiFPOri_End: return SEEK_END;
        default:
            return -1;
    }

    /* Unreachable. */
    return -1;
}
/** \endcond */


/**
 * 
 */
KiEErrorCode KI_CALL KiPlatform_OpenFile(KiTChar const *pathStr, KiEFileAccessMode mode, KiTVoid **resPtr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);
    KI_ASSERT(*pathStr != '\0',   KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr,  KiErr_OutptrParameter);

    /* (1) Convert Kira access mode to fopen() access mode. */
    char accessMode[8] = { 0 };
    KiEErrorCode errCode = KiInternal_KiraFileAccessModeToFopenFileAccessMode(mode, accessMode);
    if (errCode != KiErr_Ok)
        return errCode;

    /* (2) Open the file. */
    errno = 0;
    *resPtr = (KiTVoid *)fopen((char const *)pathStr, accessMode);
    if (*resPtr == nullptr)
        return KiNativeErrorCodeToKiraErrorCode(errno);

    return KiErr_Ok;
}

/**
 * 
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_CloseFile(KiTVoid *fHandle)  {
    if (fHandle == nullptr)
        return;

    KI_IGNORE_RETURN_VALUE(fclose((FILE *)fHandle));
}

/**
 * 
 */
KI_NATIVE extern KiTSize KI_CALL KiPlatform_GetFileSize(KiTVoid *fHandle) {
    KI_ASSERT(fHandle != nullptr, KiErr_InOutParameter);

    /* (1) Get file descriptor. */
    int const fd = fileno((FILE *)fHandle);
    if (fd == -1)
        return KI_SIZE_MAX;

    struct stat res;
    {
        if (fstat(fd, &res) != 0)
            return KI_SIZE_MAX;
    }

    return (KiTSize)res.st_size;
}

/**
 * 
 */
KI_NATIVE extern KiTOffset KI_CALL KiPlatform_GetFilePosition(KiTVoid *fHandle) {
    KI_ASSERT(fHandle != nullptr, KiErr_InOutParameter);

#if (_FILE_OFFSET_BITS == 64 || _POSIX_C_SOURCE >= 200112L)
    off_t const res = ftello((FILE *)fHandle);
#else
    off_t const res = (off_t)ftell((FILE *)fHandle);
#endif

    return (KiTOffset)res;
}

/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_SetFilePosition(KiTVoid *fHandle, KiTOffset offset, KiEFilePositionOrigin origin) {
    KI_ASSERT(fHandle != nullptr,                             KiErr_InOutParameter);
    KI_ASSERT(KiIsValidFilePositionOrigin(origin) == KI_TRUE, KiErr_InParameter);

    /* (1) Get file position origin. */
    int const ntOrigin = KiInternal_KiraFPOriginToFseekOrigin(origin);
    if (ntOrigin == -1)
        return KiErr_InParameter;

    /* (2) Set the position relative to origin. */
    errno = 0;
#if (_FILE_OFFSET_BITS == 64 || _POSIX_C_SOURCE >= 200112L)
    int const res = fseeko((FILE *)fHandle, (off_t)offset, ntOrigin);
#else
    int const res = fseek((FILE *)fHandle, (long)offset, ntOrigin);
#endif
    if (res != 0)
        return KiNativeErrorCodeToKiraErrorCode(errno);

    return KiErr_Ok;
}

/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_ReadFromFile(KiTVoid *fHandle, KiTVoid *dstBufPtr, KiTSize sizeInBytes, KiTSize *resSize) {
    KI_ASSERT(fHandle != nullptr,   KiErr_InParameter);
    KI_ASSERT(dstBufPtr != nullptr, KiErr_OutParameter);
    KI_ASSERT(sizeInBytes > 0,      KiErr_SizeParameter);
    KI_ASSERT(resSize != nullptr,   KiErr_OutParameter);

    /* EOF is not considered an error. */
    size_t const res = *resSize = fread((void *)dstBufPtr, 1, (size_t)sizeInBytes, (FILE *)fHandle);
    if (res < sizeInBytes && ferror((FILE *)fHandle) != 0)
        return KiErr_IOError;

    return KiErr_Ok;
}

/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_WriteToFile(KiTVoid *fHandle, KiTVoid const *srcBufPtr, KiTSize sizeInBytes) {
    KI_ASSERT(fHandle != nullptr,   KiErr_InOutParameter);
    KI_ASSERT(srcBufPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(sizeInBytes > 0,      KiErr_SizeParameter);

    size_t const res = fwrite((void const *)srcBufPtr, 1, (size_t)sizeInBytes, (FILE *)fHandle);
    if (res < sizeInBytes && ferror((FILE *)fHandle) != 0)
        return KiErr_IOError;

    return KiErr_Ok;
}


#endif /* KI_PLATFORM_LINUX */


