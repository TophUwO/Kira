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
 * \file  reg.c
 * \brief implements the API-, subsystem and module registry
 */


/* stdlib includes */
#include <stdlib.h>

#include <string.h>
#include <threads.h>

/* Kira includes */
#include <kira/kernel/reg.h>

#include <kira/kernel/int/htable.h>
#include <kira/kernel/int/krnlmod.h>

#include <kira/dbg/dbg.h>


/* SipHash macros */
/** \cond */
#pragma region SIPHASH-STUFF
#define ROTL(x, b) (KiTUint32)(((x) << (b)) | ((x) >> (32 - (b))))

#define U32TO8_LE(p, v)            \
    (p)[0] = (uint8_t)((v));       \
    (p)[1] = (uint8_t)((v) >> 8);  \
    (p)[2] = (uint8_t)((v) >> 16); \
    (p)[3] = (uint8_t)((v) >> 24);                                

#define U8TO32_LE(p)                                            \
    (((KiTUint32)((p)[0]))       | ((KiTUint32)((p)[1]) << 8) | \
     ((KiTUint32)((p)[2]) << 16) | ((KiTUint32)((p)[3]) << 24))     

#define SIPROUND           \
    do {                   \
        v0 += v1;          \
        v1 = ROTL(v1, 5);  \
        v1 ^= v0;          \
        v0 = ROTL(v0, 16); \
        v2 += v3;          \
        v3 = ROTL(v3, 8);  \
        v3 ^= v2;          \
        v0 += v3;          \
        v3 = ROTL(v3, 7);  \
        v3 ^= v0;          \
        v2 += v1;          \
        v1 = ROTL(v1, 13); \
        v1 ^= v2;          \
        v2 = ROTL(v2, 16); \
    } while (0)
#pragma endregion
/** \endcond */


/** \cond INTERNAL */
/**
 */
#define KI_KRNLHT_EMPTY   ((KiTVoid *)nullptr)
/**
 */
#define KI_KRNLHT_DELETED ((KiTVoid *)(-1))
/**
 */
#define KI_KRNLHT_DEFCAP  ((KiTSize)(32))
/**
 */
#define KI_KRNLHT_MAXLF   ((KiTFloat)(0.75f))


/**
 */
KI_NATIVE typedef struct KiSModuleKey {
    KiTChar *mp_mdKey;
} KiSModuleKey;

/**
 */
KI_NATIVE typedef struct KiSModuleEntry {
    KiIModule     *mp_mdRef;
    KiSMetaModule *mp_metaMdRef;
} KiSModuleEntry;

KI_NATIVE typedef struct KiSApiKey {
    KiTChar const *mp_apiKey;
    KiTChar const *mp_subsysKey;
} KiSApiKey;

/**
 */
KI_NATIVE typedef struct KiSApiEntry {
    KiSModuleEntry      const *mp_mdRef;
    KiSMetaComponent    const *mp_metaCompRef;
    KiIBase                   *mp_sgtRef;
    KiIComponentFactory       *mp_facRef;
} KiSApiEntry;

/**
 */
KI_NATIVE typedef struct KiSRegistryState {
    mtx_t             m_apiRegLock;
    mtx_t             m_mdRegLock;
    KiSKrnlHashtable *mp_apiReg;
    KiSKrnlHashtable *mp_mdReg;
} KiSRegistryState;
/**
 */
static KiSRegistryState gl_RegistryState;


/**
 */
static KiTUint64 KI_CALL KiInternal_StringHash(KiTChar const *keyPtr, KiTUint64 const hashSeed) {
    char const unsigned *ni = (char const unsigned *)keyPtr;
    char const unsigned *kk = (char const unsigned *)&hashSeed;

    KiTUint64 out;
    KiTSize const outlen = 8;
    KiTSize const inlen  = strlen(keyPtr);
    KiTUint32 v0 = 0;
    KiTUint32 v1 = 0;
    KiTUint32 v2 = UINT32_C(0x6c796765);
    KiTUint32 v3 = UINT32_C(0x74656462);
    KiTUint32 k0 = U8TO32_LE(kk);
    KiTUint32 k1 = U8TO32_LE(kk + 4);
    KiTUint32 m;
    char const unsigned *end = ni + inlen - (inlen % sizeof(KiTUint32));
    int const left = inlen & 3;
    KiTUint32 b = ((KiTUint32)inlen) << 24;
    v3 ^= k1;
    v2 ^= k0;
    v1 ^= k1;
    v0 ^= k0;
    v1 ^= 0xee;

    for (; ni != end; ni += 4) {
        m = U8TO32_LE(ni);
        v3 ^= m;

        SIPROUND; SIPROUND;

        v0 ^= m;
    }

    switch (left) {
        case 3: b |= ((KiTUint32)ni[2]) << 16;
        case 2: b |= ((KiTUint32)ni[1]) << 8;
        case 1: b |= ((KiTUint32)ni[0]);
        case 0: break;
    }

    v3 ^= b;
    SIPROUND; SIPROUND;
    v0 ^= b;
    v2 ^= 0xee;
    SIPROUND; SIPROUND;
    SIPROUND; SIPROUND;
    b = v1 ^ v3;
    U32TO8_LE((unsigned char *)&out, b);
    v1 ^= 0xdd;
    SIPROUND; SIPROUND;
    SIPROUND; SIPROUND;
    b = v1 ^ v3;
    U32TO8_LE((unsigned char *)&out + 4, b);

    return (out & 0xFFFFFFFF) ^ (out >> 32);
}

static KiTUint64 KI_CALL KiInternal_ApiRegHash(KiTVoid const *keyPtr, KiTUint64 const hashSeed) {
    if (keyPtr == nullptr)
        return 0;

    KiSApiKey const *apiKey = (KiSApiKey const *)keyPtr;

    KiTUint64 resHash = KiInternal_StringHash(apiKey->mp_apiKey, hashSeed);
    {
        if (apiKey->mp_subsysKey != nullptr)
            resHash ^= KiInternal_StringHash(apiKey->mp_subsysKey, hashSeed);

        return resHash;
    }
}

/**
 */
static KiTUint64 KI_CALL KiInternal_MdRegHash(KiTVoid const *keyPtr, KiTUint64 const hashSeed) {
    KiSModuleKey const *mdKey = (KiSModuleKey const *)keyPtr;

    return KiInternal_StringHash(mdKey->mp_mdKey, hashSeed);
}

/**
 */
static KiTBool KI_CALL KiInternal_ApiRegKeyCmp(KiTVoid const *lKeyPtr, KiTVoid const *rKeyPtr) {
    KI_ASSERT(lKeyPtr != nullptr && rKeyPtr != nullptr, KiErr_InParameter);

    KiSApiKey const *lKey = (KiSApiKey const *)lKeyPtr;
    KiSApiKey const *rKey = (KiSApiKey const *)rKeyPtr;

    /* Compare API name first. */
    if (lKey == rKey)
        return KI_TRUE;
    else if (!strcmp(lKey->mp_apiKey, rKey->mp_apiKey)) {
        /*
         * If the API part is correct, compare the subsystem part. If they are both not nullptr, return the result of
         * the comparison.
         */
        if (lKey->mp_subsysKey != nullptr && rKey->mp_subsysKey != nullptr)
            return !strcmp(lKey->mp_subsysKey, rKey->mp_subsysKey);

        /* If they are both nullptr, return true, otherwise one is nullptr, return false in this case. */
        return lKey->mp_subsysKey == nullptr && rKey->mp_subsysKey == nullptr;
    }

    /* API names are not equal. */
    return KI_FALSE;
}

/**
 */
static KiTBool KI_CALL KiInternal_MdRegKeyCmp(KiTVoid const *lKeyPtr, KiTVoid const *rKeyPtr) {
    KI_ASSERT(lKeyPtr != nullptr && rKeyPtr != nullptr, KiErr_InParameter);

    KiSModuleKey const *lKey = (KiSModuleKey const *)lKeyPtr;
    KiSModuleKey const *rKey = (KiSModuleKey const *)rKeyPtr;

    /* Simply compre the module names. */
    return lKey == rKey || !strcmp(lKey->mp_mdKey, rKey->mp_mdKey);
}

/**
 */
static KiTBool KI_CALL KiInternal_ApiRegErasePred(KiTVoid *keyPtr, KiTVoid *valPtr, KiTVoid *extraParam) {
    KI_ASSERT(keyPtr != nullptr && valPtr != nullptr, KiErr_InOutParameter);

    KiSApiKey *apiKey = (KiSApiKey *)keyPtr;
    {
        /*
         * If the API name is the same as they one we are searching for, we destroy the entire entry regardless of
         * subsystem ID.
         */
        if (!strcmp(apiKey->mp_apiKey, (KiTChar const *)extraParam)) {
            KiSApiEntry *apiEntry = (KiSApiEntry *)valPtr;
            {
                apiEntry->mp_facRef->VT->Release(apiEntry->mp_facRef);

                if (apiEntry->mp_sgtRef != nullptr)
                    apiEntry->mp_sgtRef->VT->Release(apiEntry->mp_sgtRef);
            }
        
            free(apiKey);
            free(apiEntry);
            return KI_TRUE;
        }
    }

    return KI_FALSE;
}


KiEErrorCode KI_CALL KI_KRNLMOD_INITFN(Registry)(KiTVoid *extraParam) {
    KI_UNREFERENCED_PARAMETER(extraParam);

    /* Initialize API registry and module registry. */
    KiEErrorCode errCode = KiErr_Ok;
    {
        errCode = KiKrnlHashtableCreate(
            KI_KRNLHT_DEFCAP,
            &KiInternal_MdRegHash,
            &KiInternal_MdRegKeyCmp,
            &gl_RegistryState.mp_mdReg
        );
        if (errCode == KiErr_Ok)
            errCode = KiKrnlHashtableCreate(
                KI_KRNLHT_DEFCAP,
                &KiInternal_ApiRegHash,
                &KiInternal_ApiRegKeyCmp,
                &gl_RegistryState.mp_apiReg
            );

        /*
         * Something went wrong. Simply uninitialize both because KiKrnlHashtableDestroy() can handle uninitialized
         * hashable states.
         */
        if (errCode != KiErr_Ok) {
            KiKrnlHashtableDestroy(gl_RegistryState.mp_apiReg);
            KiKrnlHashtableDestroy(gl_RegistryState.mp_mdReg);
            return errCode;
        }
    }

    /* All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KI_KRNLMOD_UNINITFN(Registry)(KiTVoid *extraParam) {
    /* Destroy both registries. */
    KiKrnlHashtableDestroy(gl_RegistryState.mp_apiReg);
    KiKrnlHashtableDestroy(gl_RegistryState.mp_mdReg);

    /* All good. */
    return KiErr_Ok;
}


KiEErrorCode KI_CALL KiKrnlRegisterModule(KiTChar const *mdName, KiIModule *mdRef) {
    KI_ASSERT(mdName != nullptr, KiErr_InParameter);
    KI_ASSERT(mdRef != nullptr, KiErr_InOutParameter);

    /* Prepare the tuple. */
    KiSModuleKey   *newKey   = calloc(1, sizeof *newKey);
    KiSModuleEntry *newEntry = calloc(1, sizeof *newEntry);
    {
        /* Abort if allocation failed. */
        if (newKey == nullptr || newEntry == nullptr) {
            free(newEntry);
            free(newKey);

            return KiErr_MemoryAllocation;
        }

        /* Populate key and entry. */
        *newKey   = (KiSModuleKey){ .mp_mdKey = (KiTChar *)mdName };
        *newEntry = (KiSModuleEntry){
            .mp_mdRef     = mdRef,
            .mp_metaMdRef = (KiSMetaModule *)mdRef->VT->QueryMetaModule(mdRef)
        };
    }

    /* Insert into registry. */
    KiEErrorCode errCode = KiKrnlHashtableInsert(gl_RegistryState.mp_mdReg, newKey, newEntry);
    if (errCode != KiErr_Ok) {
        free(newEntry);
        free(newKey);

        return errCode;
    }
    /* Add reference to module handle. */
    mdRef->VT->Acquire(mdRef);

    /* All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiKrnlUnregisterModule(KiTChar const *mdName) {
    KI_ASSERT(mdName != nullptr && *mdName != '\0', KiErr_InParameter);

    /* Extract the tuple. */
    KiSModuleKey   *mdKey;
    KiSModuleEntry *mdEntry;
    {
        KiKrnlHashtableErase(
            gl_RegistryState.mp_mdReg,
            &(KiSModuleKey const){
                .mp_mdKey = (KiTChar *)mdName
            },
            (KiTVoid **)&mdKey,
            (KiTVoid **)&mdEntry
        );
    }

    /* Destroy key and value. */
    mdEntry->mp_mdRef->VT->Release(mdEntry->mp_mdRef);
    free(mdEntry);
    free(mdKey);

    /* All good. */
    return KiErr_Ok;
}

KiEErrorCode KI_CALL KiKrnlRegisterComponent(
    KiTChar const *apiName,
    KiIModule *mdRef,
    KiSMetaComponent const *metaCompPtr,
    KiIComponentFactory *compFacRef
) {
    KI_ASSERT(apiName != nullptr || *apiName != '\0',    KiErr_InParameter);
    KI_ASSERT(mdRef != nullptr || compFacRef != nullptr, KiErr_KiOMComponentParameter);
    KI_ASSERT(metaCompPtr != nullptr,                    KiErr_InParameter);

    /* First, check if the API can even be registered. */
    if (!KiKrnlCanRegister(apiName))
        return KiErr_InvalidApi;

    /* Then, get module entry. It must be already be present. */
    KiSModuleEntry *mdEntry = KiKrnlHashtableAt(
        gl_RegistryState.mp_mdReg,
        &(KiSModuleKey const){
            .mp_mdKey = (KiTChar *)mdRef->VT->QueryMetaModule(mdRef)->m_canonicalName.mp_strPtr
        }
    );
    if (mdEntry == nullptr)
        return KiErr_ItemNotFound;

    /* Create new API key. */
    KiSApiKey *newApiKey = calloc(1, sizeof *newApiKey);
    {
        if (newApiKey == nullptr)
            return KiErr_MemoryAllocation;

        *newApiKey = (KiSApiKey){
            .mp_apiKey    = apiName,
            .mp_subsysKey = nullptr
        };
    }
    /* Create new API entry and insert tuple. */
    KiSApiEntry *newApiEntry = calloc(1, sizeof *newApiEntry);
    {
        if (newApiEntry == nullptr) {
            free(newApiEntry);

            return KiErr_MemoryAllocation;
        }

        /* Fill entry and insert it. */
        *newApiEntry = (KiSApiEntry){
            .mp_mdRef       = mdEntry,
            .mp_metaCompRef = metaCompPtr,
            .mp_sgtRef      = nullptr,
            .mp_facRef      = compFacRef
        };
        KiEErrorCode errCode = KiKrnlHashtableInsert(gl_RegistryState.mp_apiReg, newApiKey, newApiEntry);
        if (errCode != KiErr_Ok) {
            free(newApiEntry);
            free(newApiKey);

            return errCode;
        }

        /* Add a reference to each component. */
        mdRef->VT->Acquire(mdRef);
        compFacRef->VT->Acquire(compFacRef);

        /* All good. */
        return KiErr_Ok;
    }

    /* Could not create entry. */
    return KiErr_MemoryAllocation;
}

KiEErrorCode KI_CALL KiKrnlUnregisterComponent(KiTChar const *apiName) {
    KI_ASSERT(apiName != nullptr && *apiName != '\0', KiErr_InParameter);

    KiKrnlHashtableEraseIf(gl_RegistryState.mp_apiReg, &KiInternal_ApiRegErasePred, (KiTVoid *)apiName);
    return KiErr_Ok;
}

KiTBool KI_CALL KiKrnlIsRegistered(KiTChar const *apiName) {
    KI_ASSERT(apiName == nullptr && *apiName == '\0', KiErr_InParameter);

    return KiSKrnlHashtableContains(
        gl_RegistryState.mp_apiReg,
        &(KiSApiKey){
            .mp_apiKey    = apiName,
            .mp_subsysKey = nullptr
        },
        KI_DONTCARE(KiTInt64)
    );
}

KiTBool KI_CALL KiKrnlCanRegister(KiTChar const *apiName) {
    KI_ASSERT(apiName != nullptr && *apiName != '\0', KiErr_InParameter);

    /**
     * \brief contains interfaces that are not meant to be used standalone but rather as auxiliary interfaces used by
     *        the KiOM module and component system. 
     */
    static KiSStringView constexpr gl_c_AuxIfaces[] = {
        KI_MAKE_STRING_VIEW("KiIBase"),
        KI_MAKE_STRING_VIEW("KiIModule"),
        KI_MAKE_STRING_VIEW("KiIComponentFactory"),
        KI_MAKE_STRING_VIEW("KiIErrorStringifier")
    };

    /* If the given API name is in the array of forbidden interfaces, return false. */
    for (KiTSize i = 0; i < KI_COUNTOF(gl_c_AuxIfaces); i++)
        if (!strcmp(apiName, gl_c_AuxIfaces[i].mp_strPtr))
            return KI_FALSE;

    /* Could not be found, so it's fine. */
    return KI_TRUE;
}

KiEErrorCode KI_CALL KiKrnlQueryComponent(
    KiTChar const *apiId,
    KiEInstanceScope instScope,
    KiTChar const *subsysId,
    KiIBase **resPtr
) {
    KI_ASSERT(apiId != nullptr || *apiId != '\0',                             KiErr_InParameter);
    KI_ASSERT(!KI_INRANGE_EXCL(instScope, KiInstSc_None, __KiInstSc_Count__), KiErr_EnumParameter);
    KI_ASSERT(resPtr != nullptr,                                              KiErr_OutptrParameter);
    
    /* Retrieve the API entry. */
    KiSApiEntry *apiEntry = KiKrnlHashtableAt(gl_RegistryState.mp_apiReg, &(KiSApiKey const){
        .mp_apiKey    = apiId,
        .mp_subsysKey = instScope == KiInstSc_PerSubsystem
            ? subsysId
            : nullptr
    });
    if (apiEntry == nullptr && instScope == KiInstSc_PerSubsystem) {
        /*
         * If the subsystem-specific API entry could not be found, it might be that there is simply no such entry for
         * the required subsystem. In this case, we check if there is a global entry.
         */
        apiEntry = KiKrnlHashtableAt(gl_RegistryState.mp_apiReg, &(KiSApiKey const){ .mp_apiKey = apiId });
        if (apiEntry == nullptr) {
            /* Still no luck. Component is not registered. */
            *resPtr = nullptr;

            return KiErr_ItemNotFound;
        }

        /* Ok, we have a global entry. Create one for the current subsystem. */
        KiSApiKey   *newKey   = calloc(1, sizeof *newKey);
        KiSApiEntry *newEntry = calloc(1, sizeof *newEntry);
        if (newKey == nullptr || newEntry == nullptr) {
            free(newEntry);
            free(newKey);

            *resPtr = nullptr;
            return KiErr_MemoryAllocation;
        }
        /* Prepare key and entry. */
        *newKey = (KiSApiKey){
            .mp_apiKey    = apiId,
            .mp_subsysKey = subsysId
        };
        *newEntry = (KiSApiEntry){
            .mp_mdRef       = apiEntry->mp_mdRef,
            .mp_metaCompRef = apiEntry->mp_metaCompRef,
            .mp_sgtRef      = nullptr,
            .mp_facRef      = apiEntry->mp_facRef
        };

        /* Insert into registry. */
        KiEErrorCode errCode = KiKrnlHashtableInsert(gl_RegistryState.mp_apiReg, newKey, newEntry);
        if (errCode != KiErr_Ok) {
            free(newEntry);
            free(newKey);

            *resPtr = nullptr;
            return errCode;
        }
        /* Add reference to factory because we copied it. */
        newEntry->mp_facRef->VT->Acquire(newEntry->mp_facRef);
        apiEntry = newEntry;
    } else if (apiEntry == nullptr) {
        /* We queried global singleton or transient but no entry could be found. Return error. */
        *resPtr = nullptr;

        return KiErr_ItemNotFound;
    }

    /* Now we got an entry. If we want to query the singleton, we might need to instantiate it first. */
    switch (instScope) {
        case KiInstSc_Singleton:
        case KiInstSc_PerSubsystem:
            if ((*resPtr = apiEntry->mp_sgtRef) == nullptr) {
                /* Singleton does not exist. Instantiate it. */
                KiIComponentFactory *compFac = apiEntry->mp_facRef;
                {
                    KiEErrorCode eCode = compFac->VT->CreateComponent(compFac, apiId, resPtr, nullptr);

                    if (eCode != KiErr_Ok) {
                        /*
                         * Could not instantiate the component. Do not erase the subsystem entry because we might just
                         * be able to try again later. Failing to instantiate this is not automatically a system failure
                         * as the component might be optional.
                         */
                        *resPtr = nullptr;

                        return eCode;
                    }
                }

                /* Add the instance to the API entry. */
                apiEntry->mp_sgtRef = *resPtr;
            }

            break;
        case KiInstSc_Transient:
            /* Wanted transient. */
            *resPtr = (KiIBase *)apiEntry->mp_facRef;

            break;
        default:
            /*
             * Invalid. Should crash before in debug because of KI_ASSERT(). If we still get here, then I dunno ...
             * shit, ig. Better get good at debugging ...
             */
            *resPtr = nullptr;

            return KiErr_InParameter;
    }

    /* At last, we got our instance. Add a reference and return it. */
    (*resPtr)->VT->Acquire(*resPtr);
    return KiErr_Ok;
}


/** \cond */
KI_KRNLMOD_DEFINE(Registry) {
    .m_structSize = sizeof(KiSKrnlModuleInfo),
    .m_modUuid    = {},
    .m_modId      = KI_MAKE_STRING_VIEW("registry"),
    .m_modFlags   = 0,

    .mp_fnInit    = &KI_KRNLMOD_INITFN(Registry),
    .mp_fnUninit  = &KI_KRNLMOD_UNINITFN(Registry)
};
/** \endcond */


