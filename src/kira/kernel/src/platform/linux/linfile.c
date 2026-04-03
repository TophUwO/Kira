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
 * \file  windir.c
 * \brief implements Windows(R)-specific routines regarding directory handling
*/
#if (defined KI_PLATFORM_LINUX)


/* stdlib includes */
#include <stdio.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 * 
 */
static KiEErrorCode KI_CALL KiInternal_KiraFileAccessModeToFopenFileAccessMode(KiEFileAccessMode mode, char *resPtr) {
    KiTOffset currOff = 0;

    /* Stage 1: General access mode. */
    switch ((KiTUint32)mode & 0x000000FF) {
        case KiFAccMd_Read: 
        case KiFAccMd_ReadWrite:
            if (mode & KiFAccMd_MustExist && mode & KiFAccMd_MustNotExist)
                return KiErr_InvalidFileAccessMode;
            resPtr[currOff] = mode & KiFAccMd_MustExist ? 'r' : 'w';
            
            break;
        case KiFAccMd_Write:  resPtr[currOff] = 'w'; break;
        case KiFAccMd_Append: resPtr[currOff] = 'a'; break;
        default:
            return KiErr_InvalidFileAccessMode;
    }
    ++currOff;

    /* Stage 2: Content mode. */
    switch ((KiTUint32)mode & 0x00FF0000) {
        case KiFAccMd_Binary:
            resPtr[currOff] = 'b';
            
            ++currOff;
            break;
        case KiFAccMd_Text: break;
        default:
            return KiErr_InvalidFileAccessMode;
    }

    /* Stage 3: C11 extended access mode. */
    switch ((KiTUint32)mode & 0xFF000000) {
        case KiFAccMd_MustNotExist:
            if (((KiTUint32)mode & 0x000000FF) & KiFAccMd_Write)
                resPtr[currOff] = 'x';

            break;
        case KiFAccMd_MustExist: break;
        default:
            return KiErr_InvalidFileAccessMode;
    }

    return KiErr_Ok;
}
/** \endcond */


/**
 * 
 */
KiEErrorCode KI_CALL KiPlatform_OpenFile(KiTChar const *pathStr, KiEFileAccessMode mode, KiTVoid **resPtr) {
    KI_ASSERT(pathStr != nullptr, KiErr_InParameter);
    KI_ASSERT(*pathStr != '\0',   KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr,  KiErr_OutptrParameter);

    char accessMode[8] = { 0 };
    KiEErrorCode errCode = KiInternal_KiraFileAccessModeToFopenFileAccessMode(mode, accessMode);
    if (errCode != KiErr_Ok)
        return errCode;

    
}

/**
 * 
 */
KI_NATIVE extern KiTVoid KI_CALL KiPlatform_CloseFile(KiTVoid *fHandle)  {

}

/**
 * 
 */
KI_NATIVE extern KiTSize KI_CALL KiPlatform_GetFileSize(KiTVoid *fHandle) {

}

/**
 * 
 */
KI_NATIVE extern KiTOffset KI_CALL KiPlatform_GetFilePosition(KiTVoid *fHandle) {

}

/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_SetFilePosition(KiTVoid *fHandle, KiTOffset offset, KiEFilePositionOrigin origin) {

}

/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_ReadFromFile(KiTVoid *fHandle, KiTVoid *dstBufPtr, KiTSize sizeInBytes) {

}

/**
 * 
 */
KI_NATIVE extern KiEErrorCode KI_CALL KiPlatform_WriteToFile(KiTVoid *fHandle, KiTVoid const *srcBufPtr, KiTSize sizeInBytes) {

}


#endif /*  */


