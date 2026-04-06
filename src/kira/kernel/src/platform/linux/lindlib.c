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
 * \brief implements Linux-specific routines regarding dynamic library management
*/
#if (defined KI_PLATFORM_LINUX)


/* stdlib includes */
#include <string.h>

/* Linux includes */
#include <dlfcn.h>
#if (__has_include(<elf.h>))
    #include <elf.h>
#else
    #error "elf.h" is currently required. 
#endif

/* Kira includes */
#include <kira/kernel/int/platform.h>


/** \cond */
/* Use 64-bit ELF header on 64-bit targets. */
#if (defined __LP64__)
    KI_NATIVE typedef Elf64_Ehdr KiSElfHeader;
#else
    KI_NATIVE typedef Elf32_Ehdr KiSElfHeader;
#endif
/** \endcond */


/** \cond INTERNAL */
/**
 * 
 */
KI_NATIVE static KiTByte const gl_c_ELFSignature[] = { 0x7F, 'E', 'L', 'F' };


/**
 * 
 */
static KiTBool KI_CALL KiInternal_VerifyELFFileHeader(KiSElfHeader const *header) {
    KI_ASSERT(header != nullptr, KiErr_InParameter);

    KiTBool res;
    {
        /* Check ELF signature. */
        res = KI_TRUE && !memcmp(header->e_ident, gl_c_ELFSignature, sizeof gl_c_ELFSignature);
        /* Check if bitness matches with host application. */
        res = res     && header->e_ident[EI_CLASS] << 2 == sizeof(KiTVoid *);
        /* Check if the object is a dynamic library. */
        res = res     && header->e_type == ET_DYN;
    }

    return res;
}
/** \endcond */


KiTDynLibHandle KI_CALL KiPlatform_LoadLibrary(KiTChar const *libPath) {
    KI_ASSERT(libPath != nullptr, KiErr_InParameter);

    return (KiTDynLibHandle)dlopen((char const *)libPath, RTLD_NOW | RTLD_GLOBAL);
}

KiTVoid KI_CALL KiPlatform_UnloadLibrary(KiTDynLibHandle libHandle) {
    if (libHandle == nullptr)
        return;

    KI_IGNORE_RETURN_VALUE(dlclose((void *)libHandle));
}

KiSFunctionHandle KI_CALL KiPlatform_ResolveSymbol(KiTDynLibHandle libHandle, KiTChar const *symName) {
    KI_ASSERT(libHandle != nullptr, KiErr_InParameter);
    KI_ASSERT(symName != nullptr,   KiErr_InParameter);
    KI_ASSERT(*symName != '\0',     KiErr_InParameter);

    void *rslvSym = dlsym((void *)libHandle, (char const *)symName);
    if (rslvSym == nullptr)
        return (KiSFunctionHandle){ .mp_rawPtr = nullptr };

    return (KiSFunctionHandle){ .mp_rawPtr = rslvSym };
}

KiTBool KI_CALL KiPlatform_IsLibrary(KiTChar const *filePath) {
    KI_ASSERT(filePath != nullptr, KiErr_InParameter);

    /* (1) Open the file as a binary blob. */
    KiTVoid *fileHandle;
    {
        KiEErrorCode errCode = KiPlatform_OpenFile(filePath, KiFAccMd_Read | KiFAccMd_Binary, &fileHandle);
        
        if (errCode != KiErr_Ok)
            return KI_FALSE;
    }

    /* Read ELF header. */
    KiTSize resSize;
    KiSElfHeader header = { 0 };
    {
        KiEErrorCode errCode = KiPlatform_ReadFromFile(fileHandle, &header, sizeof header, &resSize);

        if (errCode != KiErr_Ok || resSize < sizeof header) {
            KiPlatform_CloseFile(fileHandle);

            return KI_FALSE;
        }
    }

    /* (3) Determine if shared object and if the format is correct. */
    KiTBool const res = KiInternal_VerifyELFFileHeader(&header);

    /* (4) Cleanup. */
    KiPlatform_CloseFile(fileHandle);
    return res;
}


#endif /* KI_PLATFORM_LINUX */


