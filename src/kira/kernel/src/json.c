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
 * \file  json.h
 * \brief defines the API for the kernel-level JSON reader
 *
 * This library only allows reading JSON because the kernel never writes it. Writing can be done via installing a
 * corresponding module in userspace.
 */


/* stdlib includes */
#if ((defined KI_PLATFORM_WINDOWS) && (!defined _CRT_SECURE_NO_WARNINGS))
    #define _CRT_SECURE_NO_WARNINGS 1
#endif /* ((defined KI_PLATFORM_WINDOWS) && (!defined _CRT_SECURE_NO_WARNINGS)) */
#include <stdlib.h>

/* external includes */
#include <kira/kernel/ext/cJSON/cJSON.h>
#include <kira/kernel/ext/cJSON/cJSON_Utils.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/json.h>
#include <kira/kernel/reg.h>

#include <kira/kernel/int/platform.h>


/** \cond INTERNAL */
/**
 */
static KiEJsonValueType KI_CALL KiInternal_JsonToKiraType(KiTInt32 cjsonTy) {
    switch (cjsonTy) {
        case cJSON_Invalid: return KiJsonValTy_Invalid;
        case cJSON_NULL:    return KiJsonValTy_Null;
        case cJSON_True:
        case cJSON_False:   return KiJsonValTy_Boolean;
        case cJSON_Number:  return KiJsonValTy_Number;
        case cJSON_String:  return KiJsonValTy_String;
        case cJSON_Array:   return KiJsonValTy_Array;
        case cJSON_Object:  return KiJsonValTy_Object;
    }

    return KiJsonValTy_Invalid;
}

/**
 */
static KiTBool KI_CALL KiInternal_IsValidValueType(KiEJsonValueType typeField) {
    return KI_INRANGE_INCL(typeField, KiJsonValTy_Invalid + 1, KiJsonValTy_Any);
}

/**
 */
static KiTBool KI_CALL KiInternal_IsTypeMismatch(KiEJsonValueType actType, KiEJsonValueType reqType) {
    if (!KiInternal_IsValidValueType(actType) || !KiInternal_IsValidValueType(reqType))
        return KI_FALSE;

    return (actType & reqType) == 0;
}
/** \endcond */


KiSJson *KI_CALL KiOpenJsonDocument(KiTChar const *filePath, KiEErrorCode *errCodePtr) {
    KI_ASSERT(filePath != nullptr,   KiErr_InParameter);
    KI_ASSERT(*filePath != '\0',     KiErr_InParameter);
    KI_ASSERT(errCodePtr != nullptr, KiErr_OutParameter);

    KiTVoid *fileHandle;
    KiTChar *rJson;
    {
        /* (1) Open file. */
        *errCodePtr = KiPlatform_OpenFile(filePath, "rb", &fileHandle);
        if (*errCodePtr != KiErr_Ok)
            return nullptr;

        /* (2) Get file size. */
        KiTSize const fileSize = KiPlatform_GetFileSize(fileHandle);
        if (fileSize == KI_SIZE_MAX) {
            KiPlatform_CloseFile(fileHandle);

            *errCodePtr = KiErr_IOError;
            return nullptr;
        }

        /* (3) Allocate buffer. */
        rJson = calloc(1, (fileSize + 1) * sizeof *rJson);
        if (rJson == nullptr) {
            KiPlatform_CloseFile(fileHandle);

            *errCodePtr = KiErr_MemoryAllocation;
            return nullptr;
        }

        /* (4) Read and close file. */
        KiTSize actReadSize;
        {
            *errCodePtr = KiPlatform_ReadFromFile(fileHandle, rJson, fileSize, &actReadSize);

            if (*errCodePtr != KiErr_Ok || actReadSize < fileSize) {
                free(rJson);
                KiPlatform_CloseFile(fileHandle);

                return nullptr;
            }
        }
        KiPlatform_CloseFile(fileHandle);
    }

    /* (5) Parse the document. */
    cJSON *newDoc = cJSON_Parse(rJson);
    free(rJson);

    if (newDoc == nullptr) {
        *errCodePtr = KiErr_LoadJsonDocument;

        return nullptr;
    }
    return (KiSJson *)newDoc;
}

KiTVoid KI_CALL KiCloseJsonDocument(KiSJson *docPtr) {
    if (docPtr == nullptr)
        return;

    cJSON_Delete((cJSON *)docPtr);
}

KiSJson *KI_CALL KiGetJsonElement(KiSJson const *elemPtr, KiTChar const *propPath) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    if (propPath == nullptr)
        return (KiSJson *)elemPtr;

    /*
     * A JSON pointer always starts with a '/'. If the first character is a forward slash, we will interpret the value
     * as a path; otherwise, it is considered to be a an object key.
     */
    return (KiSJson *)(*propPath == '/'
        ? cJSONUtils_GetPointerCaseSensitive((cJSON *)elemPtr, propPath)
        : cJSON_GetObjectItemCaseSensitive((cJSON *)elemPtr, propPath)
    );
}

KiTChar const *KI_CALL KiGetJsonElementName(KiSJson const *elemPtr) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    return ((cJSON *)elemPtr)->string;
}

KiSJson *KI_CALL KiGetPreviousJsonElement(KiSJson const *elemPtr) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    return (KiSJson *)((cJSON const *)elemPtr)->prev;
}

KiSJson *KI_CALL KiGetNextJsonElement(KiSJson const *elemPtr) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    return (KiSJson *)((cJSON const *)elemPtr)->next; 
}

KiEErrorCode KI_CALL KiSetNextJsonElement(KiSJson *elemPtr, KiTChar const *keyStr, KiSJson *nextElemPtr) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InOutParameter);

    KiTBool res;
    {
        /// TODO: check if this is good, adding an obj to an obj thinking it is an array
        if (KiGetJsonElementType(elemPtr) == KiJsonValTy_Array || keyStr == nullptr)
            res = cJSON_AddItemToArray((cJSON *)elemPtr, (cJSON *)nextElemPtr);
        else
            res = cJSON_AddItemToObject((cJSON *)elemPtr, keyStr, (cJSON *)nextElemPtr);
    }

    return res != KI_TRUE ? KiErr_JsonCouldNotAppendItem : KiErr_Ok;
}

KiTBool KI_CALL KiGetJsonElementValue(KiSJson const *elemPtr, KiSJsonValueQuery *queryPtr) {
    KI_ASSERT(elemPtr != nullptr,  KiErr_InParameter);
    KI_ASSERT(queryPtr != nullptr, KiErr_InOutParameter);

    return KiGetJsonElementValues(elemPtr, queryPtr, 1);
}

KiTBool KI_CALL KiGetJsonElementValues(
    KiSJson const *elemPtr,
    KiSJsonValueQuery *queriesArr,
    KiTSize nQueries
) {
    KI_ASSERT(elemPtr != nullptr,    KiErr_InParameter);
    KI_ASSERT(queriesArr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(nQueries > 0,          KiErr_InParameter);

    KiTBool isOk = KI_TRUE;
    {
        for (KiTSize i = 0; i < nQueries; i++) {
            /* Get the query object. */
            KiSJsonValueQuery *currQueryObj = &queriesArr[i];

            /* Get the JSON element and check type. */
            cJSON const *reqElem = (cJSON const *)KiGetJsonElement(elemPtr, currQueryObj->mp_pathStr);
            if (reqElem == nullptr || KiInternal_IsTypeMismatch(KiInternal_JsonToKiraType(reqElem->type), currQueryObj->m_reqType)) {
                if (reqElem == nullptr && currQueryObj->m_isOpt == KI_TRUE) {
                    currQueryObj->m_errCode = KiErr_JsonAttribNotFound;

                    continue;
                }

                currQueryObj->m_errCode = reqElem == nullptr
                    ? KiErr_JsonAttribNotFound
                    : KiErr_JsonAttribTypeMismatch
                ;
                isOk = KI_FALSE;
                continue;
            }

            /* Copy the value into the query object. */
            switch (currQueryObj->m_actType = KiInternal_JsonToKiraType(reqElem->type)) {
                case KiJsonValTy_Boolean: currQueryObj->m_boolValue = reqElem->valueint != KI_FALSE; break;
                case KiJsonValTy_Number:  currQueryObj->m_dblValue  = reqElem->valuedouble;          break;
                case KiJsonValTy_String:  currQueryObj->mp_strValue = reqElem->valuestring;          break;
                case KiJsonValTy_Array:
                case KiJsonValTy_Object:  currQueryObj->mp_arrValue = (KiSJson *)reqElem->child;     break;
                case KiJsonValTy_Null:    currQueryObj->mp_objValue = nullptr;                       break;
                default:
                    currQueryObj->m_errCode = KiErr_JsonInvalidValueType;

                    isOk = KI_FALSE;
            }
        }
    }

    return isOk;
}

KiEJsonValueType KI_CALL KiGetJsonElementType(KiSJson const *elemPtr) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    return KiInternal_JsonToKiraType(((cJSON const *)elemPtr)->type);
}
