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
#include <kira/kernel/json.h>
#include <kira/kernel/reg.h>

#include <kira/dbg/dbg.h>


/** \cond INTERNAL */
/**
 */
static KiEKrnlJsonValueType KI_CALL KiInternal_KrnlJsonToKiraType(KiTInt32 cjsonTy) {
    switch (cjsonTy) {
        case cJSON_Invalid: return KiKrnlJsonValTy_Invalid;
        case cJSON_NULL:    return KiKrnlJsonValTy_Null;
        case cJSON_True:
        case cJSON_False:   return KiKrnlJsonValTy_Boolean;
        case cJSON_Number:  return KiKrnlJsonValTy_Number;
        case cJSON_String:  return KiKrnlJsonValTy_String;
        case cJSON_Array:   return KiKrnlJsonValTy_Array;
        case cJSON_Object:  return KiKrnlJsonValTy_Object;
    }

    return KiKrnlJsonValTy_Invalid;
}
/** \endcond */


KiSKrnlJson *KI_CALL KiKrnlOpenJsonDocument(KiTChar const *filePath) {
    KI_ASSERT(filePath != nullptr, KiErr_InParameter);
    KI_ASSERT(*filePath != '\0',   KiErr_InParameter);

    /* Open the file. Assume valid UTF-8. */
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
    /* Parse the JSON object. */
    cJSON *newDoc = cJSON_Parse(rJson);
    free(rJson);

    return (KiSKrnlJson *)newDoc;
}

KiTVoid KI_CALL KiKrnlCloseJsonDocument(KiSKrnlJson *docPtr) {
    KI_ASSERT(docPtr != nullptr, KiErr_InOutParameter);

    cJSON_Delete((cJSON *)docPtr);
}

KiSKrnlJson *KI_CALL KiKrnlGetJsonElement(KiSKrnlJson const *elemPtr, KiTChar const *propPath) {
    KI_ASSERT(elemPtr != nullptr,   KiErr_InParameter);
    KI_ASSERT(propPath != nullptr, KiErr_InParameter);
    KI_ASSERT(*propPath != '\0',   KiErr_InParameter);

    /*
     * A JSON pointer always starts with a '/'. If the first character is a forward slash, we will interpret the value
     * as a path; otherwise, it is considered to be a an object key.
     */
    return (KiSKrnlJson *)(*propPath == '/'
        ? cJSONUtils_GetPointerCaseSensitive((cJSON *)elemPtr, propPath)
        : cJSON_GetObjectItemCaseSensitive((cJSON *)elemPtr, propPath)
    );
}

KiSKrnlJson *KI_CALL KiKrnlGetPreviousJsonElement(KiSKrnlJson const *elemPtr) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    return (KiSKrnlJson *)((cJSON const *)elemPtr)->prev;
}

KiSKrnlJson *KI_CALL KiKrnlGetNextJsonElement(KiSKrnlJson const *elemPtr) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    return (KiSKrnlJson *)((cJSON const *)elemPtr)->next; 
}

KiTBool KI_CALL KiKrnlGetJsonElementValue(KiSKrnlJson const *elemPtr, KiSKrnlJsonValueQuery *queryPtr) {
    KI_ASSERT(elemPtr != nullptr,  KiErr_InParameter);
    KI_ASSERT(queryPtr != nullptr, KiErr_InOutParameter);

    return KiKrnlGetJsonElementValues(elemPtr, queryPtr, 1);
}

KiTBool KI_CALL KiKrnlGetJsonElementValues(
    KiSKrnlJson const *elemPtr,
    KiSKrnlJsonValueQuery *queriesArr,
    KiTSize nQueries
) {
    KI_ASSERT(elemPtr != nullptr,    KiErr_InParameter);
    KI_ASSERT(queriesArr != nullptr, KiErr_InOutParameter);
    KI_ASSERT(nQueries > 0,          KiErr_InParameter);

    KiTBool isError = KI_FALSE;
    {
        for (KiTSize i = 0; i < nQueries; i++) {
            /* Get the query object. */
            KiSKrnlJsonValueQuery *currQueryObj = &queriesArr[i];

            /* Get the JSON element and check type. */
            cJSON const *reqElem = (cJSON const *)KiKrnlGetJsonElement(elemPtr, currQueryObj->mp_pathStr);
            if (reqElem == nullptr || KiInternal_KrnlJsonToKiraType(reqElem->type) != currQueryObj->m_reqType) {
                currQueryObj->m_errCode = reqElem == nullptr
                    ? KiErr_JsonAttribNotFound
                    : KiErr_JsonAttribTypeKismatch
                ;

                isError = KI_TRUE;
                continue;
            }

            /* Copy the value into the query object. */
            switch (currQueryObj->m_reqType) {
                case KiKrnlJsonValTy_Boolean: currQueryObj->m_boolValue = reqElem->valueint != KI_FALSE; break;
                case KiKrnlJsonValTy_Number:  currQueryObj->m_dblValue  = reqElem->valuedouble;          break;
                case KiKrnlJsonValTy_String:  currQueryObj->mp_strValue = reqElem->string;               break;
                case KiKrnlJsonValTy_Array:
                case KiKrnlJsonValTy_Object:  currQueryObj->mp_arrValue = (KiSKrnlJson *)reqElem->child; break;               
                default:
                    isError = KI_TRUE;
            }
        }
    }

    return isError;
}

KiEKrnlJsonValueType KI_CALL KiKrnlGetJsonElementType(KiSKrnlJson const *elemPtr) {
    KI_ASSERT(elemPtr != nullptr, KiErr_InParameter);

    return KiInternal_KrnlJsonToKiraType(((cJSON const *)elemPtr)->type);
}


