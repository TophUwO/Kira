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
 * \file  unic.c
 * \brief implements the helper routines defined in \ref unic.h
 */


/* stdlib includes */
#ifndef _CRT_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>

/* external includes */
#include <kira/kernel/ext/utf8proc/utf8proc.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/unic.h>


/** \cond INTERNAL */
/**
 */
static KiSStaticArray const gl_c_UnicWhitespaceCharacters = KI_MAKE_STATIC_ARRAY((KiTUint32 const []){
    0x0009, /* horizontal tabulation; TAB */
    0x000A, /* line-feed; LF */
    0x000B, /* line tabulation; VT */
    0x000C, /* form-feed; FF */
    0x000D, /* carriage-return; CR */
    0x0020, /* standard ASCII whitespace; SPACE */
    0x0085, /* next line; NEL */
    0x3000  /* ideographic space */
});

/**
 * \brief allows looking up the size of an encoded UTF-8 character based on its
 *        starting byte
 * \note  This LUT is based on the table found here: https://de.wikipedia.org/wiki/UTF-8#Zul%C3%A4ssige_Bytes_und_ihre_Bedeutung.
 */
static KiTByte const gl_c_Utf8CharacterSizeTable[] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


/**
 * \brief represents the UTF-8 validator state transition table
 * \note  This code is based on the UTF-8 validator by Bjoern Hoehrmann <bjoern@hoehrmann.de>, copyright (c) 2008-2009.
 *        For more details, visit http://bjoern.hoehrmann.de/utf-8/decoder/dfa.
 */
static KiTByte const gl_c_Utf8ValidationStateMachine[] = {
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,  7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
     8, 8, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    10, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 11, 6, 6, 6, 5, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,

     0, 1, 2, 3, 5, 8, 7, 1, 1, 1, 4, 6, 1, 1, 1, 1,
     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1,
     1, 2, 1, 1, 1, 1, 1, 2, 1, 2, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,
     1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1,
     1, 3, 1, 1, 1, 1, 1, 3, 1, 3, 1, 1, 1, 1, 1, 1,  1, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

/**
 * \brief  decodes a UTF-8-encoded string byte using a DFA
 *
 * \param  [in] state current state in the DFA
 * \param  [in] byte UTF-8-encoded byte to process
 *
 * \return new DFA state
 * \note   If this function returns 0, this means it could successfully read and finish a valid codepoint. This is
 *         useful for counting codepoints and also validating it. Any given UTF-8 string can be validated by feeding it
 *         all bytes into it sequentially. Once we hit the <tt>NUL</tt>-terminator, the given string is valid iff the
 *         current DFA state is 0.
 * \note   This code is based on the UTF-8 validator by Bjoern Hoehrmann <bjoern@hoehrmann.de>, copyright (c) 2008-2009.
 *         For more details, visit http://bjoern.hoehrmann.de/utf-8/decoder/dfa.
 */
static KiTUint32 inline KiInternal_Utf8ValidateChar(KiTUint32 state, KiTUint32 byte) {
    return gl_c_Utf8ValidationStateMachine[256 + state * 16 + gl_c_Utf8ValidationStateMachine[byte]];
}
/** \endcond */


KiTBool KI_CALL KiIsUnicodeWhitespace(KiTUint32 codepoint) {
    KI_ASSERT(KiIsValidUnicodeCodepoint(codepoint) == KI_TRUE, KiErr_InParameter);

    for (KiTSize i = 0; i < gl_c_UnicWhitespaceCharacters.m_elemCount; i++)
        if (codepoint == ((KiTUint32 const *)gl_c_UnicWhitespaceCharacters.mp_arrPtr)[i])
            return KI_TRUE;

    return KI_FALSE;
}

KiTBool KI_CALL KiIsValidUnicodeCodepoint(KiTUint32 codepoint) {
    return (KiTBool)utf8proc_codepoint_valid((utf8proc_int32_t)codepoint);
}

KiTUint32 KI_CALL KiToUnicodeCodepointFromUtf8(KiTChar const *u8sPtr, KiTBool *errPtr) {
    KI_ASSERT(u8sPtr != nullptr, KiErr_InParameter);

    utf8proc_int32_t res = 0x0000;
    {
        utf8proc_ssize_t const bytesRead = utf8proc_iterate((utf8proc_uint8_t const *)u8sPtr, -1, &res);

        if (bytesRead < 0) {
            *errPtr = KI_TRUE;

            return 0x0000;
        }
    }
    *errPtr = KI_FALSE;

    return (KiTUint32)res;
}

KiTBool KI_CALL KiIsValidUnicodeIdentifierStartCharacter(KiTUint32 codepoint) {
    KI_ASSERT(KiIsValidUnicodeCodepoint(codepoint) == KI_TRUE, KiErr_InParameter);

    utf8proc_category_t const cat = utf8proc_category((utf8proc_int32_t)codepoint);
    {
        utf8proc_get_property((utf8proc_int32_t)codepoint);
    }
}

KiTBool KI_CALL KiIsValidUnicodeIdentifierContinuationCharacter(KiTUint32 codepoint) {

}


KiTSize KI_CALL KiGetUtf8CharacterSize(KiTChar const *u8sPtr) {
    KI_ASSERT(u8sPtr != nullptr, KiErr_InParameter);


}

KiTBool KI_CALL KiIsUtf8StringValid(KiTChar const *u8sPtr) {
    KI_ASSERT(u8sPtr != nullptr, KiErr_InParameter);

    KiTUint32 state = 0;
    for (KiTByte const *str = (KiTByte const *)u8sPtr; *str ^ '\0'; ++str)
        state = KiInternal_Utf8ValidateChar(state, (KiTUint32)*str);

    return state == 0;
}

KiTChar *KI_CALL KiNextUtf8Character(KiTChar const *u8sStr) {
    KI_ASSERT(u8sStr != nullptr, KiErr_InParameter);

    return (KiTChar *)u8sStr + utf8proc_iterate((utf8proc_uint8_t const *)u8sStr, -1, KI_DONTCARE(utf8proc_int32_t));
}


KiEErrorCode KI_CALL KiReadAndNormalizeUtf8File(KiTChar const *filePath, KiTChar **resPtr) {
    KI_ASSERT(filePath != nullptr, KiErr_InParameter);
    KI_ASSERT(*filePath != '\0',   KiErr_InParameter);
    KI_ASSERT(resPtr != nullptr,   KiErr_OutptrParameter);

    /* Open file and get size. */
    FILE *fp = fopen(filePath, "rb");
    if (fp == nullptr) {
        *resPtr = nullptr;

        return KiErr_NoSuchFileOrDirectory;
    }
    KiTSize size;
    if (_fseeki64(fp, 0, SEEK_END) != 0) {
        *resPtr = nullptr;

        fclose(fp);
        return KiErr_IOError;
    }
    size = _ftelli64(fp);
    rewind(fp);

    /* Load file into memory. */
    KiTByte *rawBuf = malloc(size + 1), *normBuf = nullptr;
    if (rawBuf == nullptr) {
        *resPtr = nullptr;

        fclose(fp);
        return KiErr_MemoryAllocation;
    }
    if (fread(rawBuf, 1, size, fp) != size) {
        *resPtr = nullptr;

        fclose(fp);
        return KiErr_IOError;
    }
    rawBuf[size] = '\0';
    fclose(fp);

    /* Decompose and validate. */
    KiTInt64 normSize = utf8proc_decompose(rawBuf, 0, nullptr, 0, UTF8PROC_NULLTERM);
    if (normSize < 0) {
        *resPtr = nullptr;

        free(rawBuf);
        return KiErr_EncodingError;
    }
    normBuf = realloc(rawBuf, normSize * sizeof(utf8proc_int32_t));
    if (normBuf == nullptr) {
        *resPtr = nullptr;

        free(rawBuf);
        return KiErr_MemoryReallocation;
    }

    /* Normalize. All strings returned by this function are NFC. */
    if (utf8proc_reencode((utf8proc_int32_t *)normBuf, normSize, UTF8PROC_STABLE | UTF8PROC_COMPOSE) < 0) {
        *resPtr = nullptr;

        free(normBuf);
        return KiErr_EncodingError;
    }

    *resPtr = (KiTChar *)normBuf;
    return KiErr_Ok;
}


