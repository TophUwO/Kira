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
 * \file  winfile.c
 * \brief implements Windows-specific basic file handling routines
 */


#if (defined KI_PLATFORM_WINDOWS)
/* stdlib includes */
#include <stdio.h>

#include <sys/stat.h>

/* Windows includes */
#include <windows.h>

/* Kira includes */
#include <kira/kcm.h>
#include <kira/dbg.h>

#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
static inline KiEErrorCode KI_CALL KiInternal_FormatOpenModeString(KiTChar const *modeStr, wchar_t *resPtr) {
    KI_ASSERT(modeStr != nullptr, KiErr_InParameter);
    KI_ASSERT(*modeStr != '\0',   KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr,  KiErr_OutParameter);

    /*
     * This function assumes that it is a standard fopen() mode string which as of now (C23) only contains ASCII
     * characters.
     */
    KiTOffset i;
    for (i = 0; modeStr[i] != '\0'; i++) {
        if ((KiTByte)modeStr[i] >= 0x80)
            return KiErr_EncodingError;

        resPtr[i] = (wchar_t)modeStr[i];
    }

    /* All good. */
    resPtr[i] = L'\0';
    return KiErr_Ok;
}

/**
 */
static inline KiTInt32 KI_CALL KiInternal_CvtToNativeFilePositionOrigin(KiEFilePositionOrigin origin) {
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


KiEErrorCode KI_CALL KiPlatform_OpenFile(KiTChar const *pathStr, KiTChar const *modeStr, KiTVoid **resPtr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);
    KI_ASSERT(*pathStr != '\0',   KiErr_InParameter);
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);
    KI_ASSERT(*pathStr != '\0',   KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr,  KiErr_OutptrParameter);

    /* (1) Convert file name to native encoding. */
    WCHAR *u16Path = KiPlatform_CreateFromKiraEncoding(pathStr, -1, KI_DONTCARE(KiTSize), KI_DONTCARE(KiTSize));
    if (u16Path == nullptr) {
        *resPtr = nullptr;

        return KiErr_EncodingError;
    }

    /* (2) Open the file. */
    wchar_t accessMode[8] = { 0 };
    {
        /* (2.1) Convert numeric open mode to string. */
        KiEErrorCode errCode = KiInternal_FormatOpenModeString(modeStr, &accessMode[0]);
        if (errCode != KiErr_Ok) {
            KiPlatform_FreeString((KiTVoid *)u16Path);

            *resPtr = nullptr;
            return errCode;
        }

        /* (2.2) Open or create the file. */
        errno_t const res = _wfopen_s((FILE **)resPtr, (wchar_t const *)u16Path, accessMode);
        if (res != 0) {
            KiPlatform_FreeString((KiTVoid *)u16Path);

            *resPtr = nullptr;
            return KiNativeErrorCodeToKiraErrorCode((KiTInt32)res);
        }
    }

    /* (3) Clean up. */
    KiPlatform_FreeString((KiTVoid *)u16Path);
    return KiErr_Ok;
}

KiTVoid KI_CALL KiPlatform_CloseFile(KiTVoid *fHandle) {
    KI_ASSERT(fHandle != nullptr, KiErr_InOutParameter);

    fclose((FILE *)fHandle);
}

KiTSize KI_CALL KiPlatform_GetFileSize(KiTVoid *fHandle) {
    KI_ASSERT(fHandle != nullptr, KiErr_InParameter);

    if (ferror((FILE *)fHandle) != 0)
        return KI_SIZE_MAX;

    struct _stat64 fileStat;
    {
        int const res = _fstat64(_fileno((FILE *)fHandle), &fileStat);

        if (res == -1)
            return KI_SIZE_MAX;
    }

    return (KiTSize)fileStat.st_size;
}

KiTOffset KI_CALL KiPlatform_GetFilePosition(KiTVoid *fHandle) {
    KI_ASSERT(fHandle != nullptr, KiErr_InParameter);
    
    /* (1) Handle stream errors. */
    if (ferror(fHandle) != 0)
        return -1;

    /* (2) Get the file position. */
    errno = 0;
    {
        KiTOffset const res = (KiTOffset)_ftelli64((FILE *)fHandle);
        if (res == -1 || errno != 0)
            return -1;

        return res;
    }
}

KiEErrorCode KI_CALL KiPlatform_SetFilePosition(KiTVoid *fHandle, KiTOffset offset, KiEFilePositionOrigin origin) {
    KI_ASSERT(fHandle != nullptr,                  KiErr_InOutParameter);
    KI_ASSERT(KiIsValidFilePositionOrigin(origin), KiErr_InParameter);

    /* (1) Convert origin to native origin. */
    int const ntOrigin = (int)KiInternal_CvtToNativeFilePositionOrigin(origin);
    if (ntOrigin == -1)
        return KiErr_InParameter;

    /* (2) Set file position. */
    errno = 0;
    {
        int const res = _fseeki64((FILE *)fHandle, (long)offset, ntOrigin);

        if (res != 0)
            return KiNativeErrorCodeToKiraErrorCode((KiTInt32)errno);
    }

    /* All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiPlatform_ReadFromFile(KiTVoid *fHandle, KiTVoid *dstBufPtr, KiTSize sizeInBytes, KiTSize *resSize) {
    KI_ASSERT(fHandle != nullptr,   KiErr_InParameter);
    KI_ASSERT(dstBufPtr != nullptr, KiErr_OutParameter);
    KI_ASSERT(sizeInBytes > 0,      KiErr_SizeParameter);
    KI_ASSERT(resSize != nullptr,   KiErr_OutParameter);

    errno = 0;
    {
        /* (1) Read data from the file. */
        size_t const res = fread(dstBufPtr, 1, sizeInBytes, (FILE *)fHandle);

        /* (2) Check for errors. EOF is not considered to be an error. */
        if (res < sizeInBytes && ferror((FILE *)fHandle)) {
            *resSize = 0;

            return errno != 0 ? KiNativeErrorCodeToKiraErrorCode((KiTInt32)errno) : KiErr_IOError;
        }
        *resSize = (KiTSize)res;
    }

    /* All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiPlatform_WriteToFile(KiTVoid *fHandle, KiTVoid const *srcBufPtr, KiTSize sizeInBytes) {
    KI_ASSERT(fHandle != nullptr,   KiErr_InOutParameter);
    KI_ASSERT(srcBufPtr != nullptr, KiErr_InParameter);
    KI_ASSERT(sizeInBytes > 0,      KiErr_SizeParameter);

    errno = 0;
    {
        /* (1) Write data to file. */
        size_t const res = fwrite(srcBufPtr, 1, sizeInBytes, (FILE *)fHandle);

        /* (2) Check for errors. If we wrote less than we wanted, there must have been an error. */
        if (res < sizeInBytes)
            return errno != 0 ? KiNativeErrorCodeToKiraErrorCode((KiTInt32)errno) : KiErr_IOError;
    }

    /* All good. */
    return KiErr_Ok;
}
#endif /* (defined KI_PLATFORM_WINDOWS) */
