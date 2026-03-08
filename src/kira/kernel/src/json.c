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
 * \file  json.h
 * \brief defines the API for the kernel-level JSON reader
 *
 * This library only allows reading JSON because the kernel never writes it. Writing can be done via installing a
 * corresponding module in userspace.
 */


/* stdlib includes */
#include <stdio.h>
#include <stdlib.h>

/* external includes */
#include <kira/kernel/ext/cJSON/cJSON.h>
#include <kira/kernel/ext/cJSON/cJSON_Utils.h>

/* Kira includes */
#include <kira/dbg.h>

#include <kira/kernel/json.h>
#include <kira/kernel/reg.h>


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
static KiTBool KI_CALL KiInternal_IsValidValueType(KiTInt32 typeId) {
    return KI_INRANGE_EXCL(typeId, KiJsonValTy_Invalid, __KiJsonValTy_Count__);
}

/**
 */
static KiTBool KI_CALL KiInternal_IsTypeMismatch(KiEJsonValueType actType, KiEJsonValueType reqType) {
    if (!KiInternal_IsValidValueType(actType) || !KiInternal_IsValidValueType(reqType))
        return KI_FALSE;

    return (actType & reqType) == 0;
}
/** \endcond */


KiSJson *KI_CALL KiOpenJsonDocument(KiTChar const *filePath) {
    KI_ASSERT(filePath != nullptr, KiErr_InParameter);
    KI_ASSERT(*filePath != '\0',   KiErr_InParameter);

    KiTChar *rJson;
    {
        /* Open file. */
        FILE *fPointer;
        if (fopen_s(&fPointer, filePath, "rb") != 0)
            return nullptr;
        /* Get file size. */
        _fseeki64(fPointer, 0, SEEK_END);
        KiTSize const fSize = _ftelli64(fPointer);
        rewind(fPointer);

        /* Allocate array. */
        rJson = malloc((fSize + 1) * sizeof *rJson);
        if (rJson == nullptr) {
            fclose(fPointer);

            return nullptr;
        }
        /* Read and close file. */
        fread_s(rJson, (fSize + 1) * sizeof *rJson, 1, fSize, fPointer);
        fclose(fPointer);
    }
    cJSON *newDoc = cJSON_Parse(rJson);
    free(rJson);

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


