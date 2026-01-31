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
 * \file  kcm.h
 * \brief defines the main symbols, macros, data-structures for Kira's component framework implementing the Kira Object
 *        Model (KiOM)
 *
 * Kira's API is entirely interface-based. An \c interface in Kira's terms is a data-structure containing only function
 * pointers. \c Components are implementations of \c interfaces which implement the contract the interface specifies.
 * Components can implement any number of interfaces but not the same interface more than once. The techniques used for
 * Kira's APIs resemble those used by, for example, Microsoft's COM technology.
 */


#pragma once

/* Kira includes */
#include <kira/util.h>


/**
 * \def   KI_METADATA(...)
 * \brief declares a piece of metadata for the current entity
 *
 * \par Metadata
 *   Kira uses a special pre-compile step, invoking the \c KiPP tool in order to generate metadata and boilerplate
 *   code for the current module. \c KI_METADATA provides \c KiPP with most of the necessary information to generate
 *   that code.
 *
 * \par Scopes
 *   The following lists all possible scopes recognized by KiPP. Note that the scope names are case-insensitive. The
 *   standard, however, is to write them in all caps.
 *   <ol>
 *    <li>\c module - specify metadata for the module</li>
 *    <li>\c component - specify metadata for a component</li>
 *    <li>\c interface - specify metadata for an interface</li>
 *   </ol>
 */
#define KI_METADATA(...)
/**
 * \def   KI_ERRORCODE(numCode, ...)
 * \brief defines a module-wide error code
 *
 * Since Kira employs a loosely-coupled, fully modular architecture, it cannot centrally define error codes. Instead,
 * every module can describe the error codes it defines, associating the numeric error code to, optionally, brief and
 * detailed descriptions. That's what this macro is for.
 *
 * \param numCode numeric error code that is to be described
 * \note  \li Apart from \c numCode, <tt>KI_ERRORCODE(...)</tt> allows for up to two more parameters. The second
 *            parameter, if present, is interpreted as the error code's \e brief description while the third parameter,
 *            if present, is interpreted as the error code's \e detailed description.
 * \note  \li It is possible to omit the \e brief description but have a \e detailed description by setting the brief
 *            description to an empty string <tt>""</tt> or \c null (unescaped, case-insensitive).
 */
#define KI_ERRORCODE(numCode, ...)

/**
 * \def   KI_INTERFACE(iId)
 * \brief auxiliary macro used to define a new interface
 *
 * An interface is a data-structure that consists only of function pointers. Use this macro to insert the interface
 * declaration as well as necessary meta-information on the new interface.
 * When creating new interfaces, a few rules must be abided by:
 *  \li Every derived interface must contain all methods exposed by <tt>KiIBase</tt>.
 *  \li Every derived interface's first three methods must exactly reflect those found in <tt>KiIBase</tt>. That means,
 *      the order of declaration, the name, and the prototypes must be identical barring the pointer type of the \c self
 *      parameters.
 *
 * \param iId name of the new interface, non-escaped
 */
#define KI_INTERFACE(iId)                                          \
    KI_NATIVE typedef struct iId iId;                              \
    KI_NATIVE struct iId { struct __##iId##_VTable__ const *VT; }; \
    KI_NATIVE struct __##iId##_VTable__

/**
 * \def   KI_COMPONENT(clsName)
 * \brief introduces a Kira component definition
 *
 * In KiOM, \e components are basically the equivalent of \e classes in that they implement \e interfaces. Therefore,
 * whenever there is an occurrence of \c KI_COMPONENT(), a \c {} block must follow which follows the rules for standard
 * C struct definitions, describing the inner layout of the component. There are not many hard requirements as \c KiPP
 * is able to deal with most curiosities. However, the first members of the struct must be the interfaces that the
 * component implements, all of which must contain the three base methods of \c KiIBase at their start. Refer to the
 * <tt>KiOM programming guide</tt> for an in depth discussion on the relevant concepts.
 *
 * \param compName name of the component, unescaped
 */
#define KI_COMPONENT(compName)        \
    typedef struct compName compName; \
    struct compName
/**
 * \defgroup KiPPAnnos KiPP Component Annotations
 * \brief    contains annotations for use inside the <tt>KI_COMPONENT(...) { ... }</tt> definition
 */
/** @{ */
/**
 * \def   KI_IMPLEMENTS(iId)
 * \brief auxiliary macro which inserts a pointer to the VTable of the given interface into a structure definition
 * 
 * This macro is useful when creating components that implement interfaces.
 */
#define KI_IMPLEMENTS(iId)                \
    struct iId KI_CONCAT2(iId, _Iface);   \
    KiSUuid const *KI_CONCAT2(iId, _Iid)
/**
 * \def   KI_REQUIRES(iId) [X]*
 * \brief declares that the component this directive resides in depends on the interface named \c iId on the terms
 *        denoted by an arbitrary sequence of \c X (requirement annotations)
 * \param iId name of the interface
 * \note  \c X must be a sequence of KI_* macros. The following options are recognized:
 *         <ul>
 *          <li>\c KI_OPTIONAL - optional requirement; component can react to this requirement not being fulfilled</li>
 *          <li>\c KI_REQUIRED - mandatory requirement; default</li>
 *          <li>\c KI_SINGLETON - requires shared (singleton) instance; default</li>
 *          <li>\c KI_TRANSIENT - requires transient instance</li>
 *          <li>\c KI_SUBSYSTEM - requires singleton instance scoped to the subsystem the component is associated with</li>
 *         </ul>
 * \note The way component requirements are stored is as follows:<br>
 *        <ol>
 *         <li>When a component requirement directive is annotated with \c KI_SINGLETON, a pointer to this instance is 
 *             passed directly to the requiring component. This means the union member with suffix \c *_Inst will be
 *             valid and point to an actual instance of a component implementing the required interface. Thus, it can be
 *             used directly without the need to do \c Acquire() or \c QueryInterface().</li>
 *         <li>When a component requirement directive is annotated with \c KI_TRANSIENT, meaning that the component can
 *             request multiple instances of said component, a factory able to create these components is instead passed
 *             instead; in this case, the union member with suffix \c *_Fac will be valid and directly useable.</li>
 *        </ol>
 */
#define KI_REQUIRES(iId)                                    \
    KI_UNION(                                               \
        struct iId                 *KI_CONCAT2(iId, _Inst); \
        struct KiIComponentFactory *KI_CONCAT2(iId, _Fac);  \
    );                                                      \
    KiSResolvedRequirement const *KI_CONCAT2(iId, _Req);

/**
 * \def   KI_SINGLETON
 * \brief annotation picked up by \c KiPP denoting an interface requirement as a singleton, meaning that the component
 *        requires a global component implementing the given interface, sharing it with other components which also
 *        require it as a singleton
 * \see   KI_TRANSIENT, KI_OPTIONAL
 * \note  When no \c KI_TRANSIENT or \c KI_SUBSYSTEM is specified, \c KI_SINGLETON is automatically implied.
 * \note  When this annotation is specified, the component loader will inject a reference to the instance into the
 *        component.
 * \note  This annotation is mutually exclusive with \c KI_TRANSIENT and \c KI_SUBSYSTEM but can appear alongside
 *        \c KI_OPTIONAL.
 * \note  Like all annotations, they must appear after <tt>KI_REQUIRES(...)</tt>.
 */
#define KI_SINGLETON
/**
 * \def   KI_TRANSIENT
 * \brief annotation picked up by \c KiPP denoting an interface requirement is 'transient' which means the component
 *        might use more than one instance of the component implementing the required interface
 * \see   KI_SINGLETON, KI_OPTIONAL
 * \note  When this annotation is specified, the component loader will inject a reference to the \c KiIComponentFactory
 *        capable of instantiating the component implementing the required interface.
 * \note  Unlike \c KI_SINGLETON, \c KI_TRANSIENT is \b not implied when there is no \c KI_SINGLETON but must be
 *        explicitly specified.
 * \note  This annotation is mutually exclusive with \c KI_SINGLETON and \c KI_SUBSYSTEM but can appear alongside
 *        \c KI_OPTIONAL.
 * \note  Like all annotations, they must appear after <tt>KI_REQUIRES(...)</tt>.
 */
#define KI_TRANSIENT
/**
 */
#define KI_INSTANCE
/**
 * \def   KI_OPTIONAL
 * \brief annotation picked up by \c KiPP denoting an interface requirement as 'optional', that is, the component
 *        requiring it can do without and is able to detect the presence of the requirement whenever it's needed
 * \see   KI_TRANSIENT, KI_OPTIONAL
 * \note  When this annotation is specified, and no interface implementing the required component is available, the
 *        component loader sets the component's pointer to \c nullptr. The requiring component must be aware of that and
 *        verify that the component is available before use. The requirement can be fulfilled at a later point in time
 *        manually once a component implementing the required interface becomes available (through injection into the
 *        process.)
 * \note  This annotation can appear alongside \c KI_SINGLETON, \c KI_TRANSIENT, and \c KI_SUBSYSTEM.
 * \note  Like all annotations, they must appear after <tt>KI_REQUIRES(...)</tt>.
 */
#define KI_OPTIONAL
/**
 * \def   KI_SUBSYSTEM
 * \brief annotation picked up by \c KiPP denoting that the interface requirement is scoped to the subsystem, that is,
 *        the instance is effectively \c KI_SINGLETON but only for the KiOM components inside the same subsystem
 * \see   KI_SINGLETON, KI_OPTIONAL
 * \note  Unlike \c KI_SINGLETON, \c KI_SUBSYSTEM is \b not implied when there is no \c KI_SUBSYSTEM but must be
 *        explicitly specified.
 * \note  This annotation is mutually exclusive with \c KI_SINGLETON and \c KI_TRANSIENT but can appear alongside
 *        \c KI_OPTIONAL. 
 * \note  Like all annotations, they must appear after <tt>KI_REQUIRES(...)</tt>.
 */
#define KI_SUBSYSTEM

/**
 */
#define KI_NAMED(ifaceName)


/**
 * \def   KI_PUBLIC
 * \brief annotation picked up by \c KiPP denoting an interface implementation as a \e public interface, meaning the
 *        component can be made available globally under this interface
 * \see   KI_PRIVATE
 * \note  This annotation is mutually exclusive with \c KI_PRIVATE.
 * \note  This annotation is not implied when not specified. Thus, a public interface must be explicitly marked as such.
 * \note  Like all annotations, they must appear after <tt>KI_IMPLEMENTS(...)</tt>.
 */
#define KI_PUBLIC
/**
 * \def   KI_PRIVATE
 * \brief denotes an interface implemented by the current component that is to be hidden from the users of the component
 *
 * The main side-effect of annotating an \c KI_IMPLEMENTS(...) with \c KI_PRIVATE is that said interface cannot be
 * queried using \c KiIBase::QueryInterface(). It also cannot be discovered by examining the component's
 * \c KiSComponentMetadata. From this angle, it is the exact opposite of \c KI_PUBLIC. However, from the component's
 * point of view, it's a perfectly functional and internally usable interface.
 *
 * \see   KI_PUBLIC
 * \note  This annotation is mutually exclusive with \c KI_PUBLIC.
 * \note  This annotation is not implied when not specified. Thus, a public interface must be explicitly marked as such.
 * \note  Like all annotations, they must appear after <tt>KI_IMPLEMENTS(...)</tt>.
 */
#define KI_PRIVATE
/** @} */

/**
 * \defgroup KiIfaceAnnos KI_INTERFACE annotations
 * \brief    annotations for use in and with KI_INTERFACE directives
 */
/** @{ */
/**
 */
#define KI_AUXILIARY
/**
 */
#define KI_BUILTIN
/**
 */
#define KI_DEPRECATED
/**
 */
#define KI_REMOVED

/**
 */
#define KI_EXTENDS(...)
/**
 */
#define KI_EXCLUDES(...)
/**
 */
#define KI_INCLUDES(...)
/**
 */
#define KI_INCLUDES_ANY(...)
/**
 */
#define KI_INCLUDES_ONE(...)

/**
 * \def   KI_BASE(iface)
 * \brief injects the methods of \c KiIBase; you can use this as a convenience macro
 */
#define KI_BASE(iface)                                                                             \
    KI_STRUCT(                                                                                     \
        KiEErrorCode (KI_CALL *QueryInterface)(iface *self, KiTChar const *iId, KiTVoid **resPtr); \
        KiTInt32 (KI_CALL *Acquire)(iface *self);                                                  \
        KiTInt32 (KI_CALL *Release)(iface *self);                                                  \
        KiSMetaComponent *(KI_CALL *GetMetaComponent)(iface *self);                                \
    );
/** @} */

/**
 * \def   KI_CONTAINEROF(comp, iface)
 * \brief retrieve the component's base address in order to access its internal state, etc.
 * \param comp name of the component, unescaped, in its <em>canonical name</em>; e.g., \c KiCHashtable
 * \param iface name of the interface the current method belongs to, unescaped, in its <em>canonical name</em>; e.g.,
 *              \c KiIBase
 * \note  The return value is casted to the pointer-type of \c comp, so you can use the \c auto keyword (provided you
 *        are in C23 mode) to set the type of your \c this pointer, e.g.:
 *        \code{.cpp}
 *            auto *this = KI_CONTAINEROF(KiCExample, KiICurrentInterface);
 *            ...
 *            // Can now use internal state of "KiCExample".
 *            this->mp_vecObj ...
 *        \endcode
 */
#define KI_CONTAINEROF(comp, iface) ((struct comp *)(                                        \
        ((KiTByte *)((struct iface *)(self))) - KI_OFFSETOF(comp, KI_CONCAT2(iface, _Iface)) \
    ))


/** \cond */
KI_NATIVE typedef struct KiIBase             KiIBase;
KI_NATIVE typedef struct KiIComponentFactory KiIComponentFactory;
KI_NATIVE typedef struct KiIModule           KiIModule;
KI_NATIVE typedef struct KiSBuildInformation KiSBuildInformation;
/** \endcond */

/**
 * \typedef KiTRefCount
 * \brief   special type for reference count of KiOM objects
 */
KI_NATIVE typedef _Atomic(KiTInt32) KiTRefCount;

/**
 * \typedef KiFQueryModule
 * \brief   describes the prototype of the \c KiIModule query function
 *
 * When a module is loaded, the module loader will invoke the \c KiGetModule() function defined in the module, using it
 * to query the module descriptor which is then used to determine all other properties substantial to module loading and
 * initialization. Make sure your version of the function is properly exported. If you are using the
 * \c KiPP (<b>S</b>ha<b>k</b>ai <b>P</b>re<b>p</b>rocessor), this is automatically handled for you.
 * 
 * \return  The return value of the callback must be a reference to the \c KiIModule instance in which the
 *          function lives.
 */
KI_NATIVE typedef KiIModule *(KI_CALL *KiFQueryModule)(KiTVoid);
/**
 * \typedef KiFGetBuildInformation
 * \brief   signature of the function that's used to expose the static build information for a given target to the rest
 *          of the system
 * \note    Every target that is part of Kira must expose this method. If it does not, it cannot be loaded.
 */
KI_NATIVE typedef KiSBuildInformation const *(KI_CALL *KiFGetBuildInformation)(KiTVoid);


/**
 * \enum  KiEModuleFlags
 * \brief represents miscellaneous KiOM module flags
 */
KI_NATIVE typedef enum KiEModuleFlags {
    KiMdFl_Disabled = 1 << 0, /**< whether or not the module is disabled or not (will not be loaded if disabled) */
    KiMdFl_Autogen  = 1 << 1  /**< whether or not KiPP was invoked for this module */
} KiEModuleFlags;

/**
 * \enum  KiEComponentFlags
 * \brief represents miscellaneous KiOM component flags
 */
KI_NATIVE typedef enum KiEComponentFlags {
    KiClsFl_Disabled  = 1 << 0, /**< whether or not the component is disabled (will not be loaded if disabled) */
    KiClsFl_Autogen   = 1 << 1, /**< whether or not KiPP was invoked for this component */
    KiClsFl_HotReload = 1 << 2, /**< whether or not the component can be hot-reloaded */
    KiClsFl_Private   = 1 << 3  /**< whether or not the component is not supposed to be handled by the API registry */
} KiEComponentFlags;

/**
 */
KI_NATIVE typedef enum KiERequiredInterfaceFlags {
    KiReqFl_Singleton = 1 << 0,
    KiReqFl_Transient = 1 << 1,
    KiReqFl_Optional  = 1 << 2
} KiERequiredInterfaceFlags;

/**
 */
KI_NATIVE typedef enum KiEProvidedInterfaceFlags {
    KiProvFl_Normal  = 1 << 0,
    KiProvFl_Public  = 1 << 1,
    KiProvFl_Private = 1 << 2,

    KiProvFl_Default   = KiProvFl_Normal
} KiEProvidedInterfaceFlags;

/**
 * \enum  KiEInstanceScope
 * \brief represents the scope of a particular instance (-dependency)
 *
 * In \c KiOM, interface requirements can be \e scoped. This means that depending on the instance's context, a specific
 * instance is used. There are currently three scopes, namely
 * <ul>
 *  <li>\e singleton,</li>
 *  <li>\e transient,</li>
 *  <li>and \e subsystem.</li>
 * </ul>
 * \e Singleton means that there is a singular, global instance that is centrally managed and can be accessed by any
 * KiOM component across subsystems and threads. Such components must generally be protected by a synchronization
 * primitive such as a \e mutex, or <em>reader-writer lock</em> unless they do not modify shared state.<br>
 * \e Transient instances are generally not shared across subsystems and are usually owned by a specific instance. For
 * instance, data-structures like maps, trees, etc. are usually used by an KiOM component to internally manage state.
 * Such instances are private to the respective owner and protected by the owner against contention if needed.<br>
 * KiOM components scoped to a specific \e subsystem are generally equivalent to \e singletons. However, they cannot be
 * accessed by literally \e any other KiOM component but only those which are associated with the subsystem the instance
 * was created for. There is no way for a subsystem-specific instance to be shared across subsystems. However, it may be
 * shared across threads, thus, synchronization is potentially necessary.
 * 
 * \note  In general, KiOM components should be designed to be context-agnostic, meaning that they can handle any usage
 *        context appropriately. However, it may be useful for some components to force a specific usage context. For
 *        this reason, in the component's constructor, the intended usage context is accessible. The instance can then
 *        decide if it is compatible with the specific usage context and if it isn't, it can simply return an error; 
 *        i.e., \c KiErr_UnsupportedUsageCtxt. However, this kind of undermines the principle of context-agnosticity and
 *        should thus only be used if necessary for reasons like performance or semantic usefulness.
 */
KI_NATIVE typedef enum KiEInstanceScope {
    KiInstSc_None,         /**< no/invalid instance context */
    KiInstSc_Singleton,    /**< singleton; global instance */
    KiInstSc_Transient,    /**< transient; mutiple instances possible */
    KiInstSc_PerSubsystem, /**< object is scoped to subsystem */

    __KiInstSc_Count__     /**< *only used internally* */
} KiEInstanceScope;

/**
 */
KI_NATIVE typedef enum KiEInterfaceFlags {
    KiIfaceFl_None       = 0,

    KiIfaceFl_Autogen    = (1 << 0),
    KiIfaceFl_Auxiliary  = (1 << 1),
    KiIfaceFl_Deprecated = (1 << 2),
    KiIfaceFl_Removed    = (1 << 3)
} KiEInterfaceFlags;

/**
 */
KI_NATIVE typedef enum KiEEndianness {
    KiEnd_Unknown,

    KiEnd_Little,
    KiEnd_Big,

    KI_ENUM_COUNT(KiEnd)
} KiEEndianness;

/**
 * \enum  KiEErrorCode
 * \brief represents numeric error codes used by Kira
 */
KI_NATIVE typedef enum KiEErrorCode {
    KiErr_Ok                = 0x7FFF0000, /**< no error */
    KiErr_Unknown,                        /**< unknown error condition */
    KiErr_NoOperation,                    /**< function did nothing */
    KiErr_ManuallyAborted,                /**< operation was manually aborted by user or callback */
    KiErr_NotImplemented,                 /**< returned if the calling function is not (yet) implemented */
                        
    KiErr_InParameter,
    KiErr_OutParameter,
    KiErr_InOutParameter,
    KiErr_CallbackParameter,
    KiErr_InptrParameter,
    KiErr_OutptrParameter,
    KiErr_InOutptrParameter,
    KiErr_SelfParameter,
    KiErr_KiOMComponentParameter,
    KiErr_EnumParameter,
    KiErr_SizeParameter,
    KiErr_IndexParameter,
    KiErr_NoInstance,
    KiErr_NoMetaClasses,
    KiErr_InterfaceNotImplemented,
    KiErr_MemoryAllocation,
    KiErr_MemoryReallocation,
    KiErr_CapLimitExceeded,
    KiErr_ContainerFull,
    KiErr_ItemNotFound,
    KiErr_ItemAlreadyExists,
    KiErr_SynchInit,
    KiErr_RangeError,
    KiErr_IndexError,
    KiErr_DlibLoadLibrary,
    KiErr_DlibUnloadLibrary,
    KiErr_DlibResolveSymbol,
    KiErr_IncorrectInitState,
    KiErr_UnsupportedUsageCtxt,
    KiErr_IllegalObjectState,
    KiErr_InvalidApi,
    KiErr_UnknownModule,
    KiErr_UnknownErrorCode,
    KiErr_InvalidEnumContextType,
    KiErr_NoSuchFileOrDirectory,
    KiErr_InsufficientFilePerms,
    KiErr_EntityIsADirectory,
    KiErr_EntityIsNotADirectory,
    KiErr_IOError,
    KiErr_EncodingError,
    KiErr_LoadJsonDocument,
    KiErr_IllegalSystemState,
    KiErr_GetSystemPath,
    KiErr_PickProfile,
    KiErr_JsonAttribNotFound,
    KiErr_JsonAttribTypeMismatch,
    KiErr_IllegalMemoryPoolState,
    KiErr_VirtualReserve,
    KiErr_VirtualCommit,
    KiErr_VirtualUncommit,
    KiErr_VirtualFree,
    KiErr_NoConfigDirectory,

    KI_ENUM_COUNT(KiErr)
} KiEErrorCode;


/**
 * \struct KiSLegalInformation
 * \brief  represents the Kira metadata legal information structure
 *
 * Many entities in Kira can have metadata attached to them, namely \c Interfaces, \c Components, \c Modules,
 * <tt>Test Suites</tt> and <tt>Test Cases</tt>. All of them can contain information regarding the author and legal
 * constraints when using the entity. This structure models the default Kira legal information object.
 */
KI_NATIVE typedef struct KiSLegalInformation {
    KiTSize       m_structSize; /**< size of this structure, in bytes */
    KiSStringView m_author;     /**< author identification (name, etc.) */
    KiSStringView m_contact;    /**< author contact information */
    KiSStringView m_license;    /**< license string identifier */
    KiSStringView m_copyright;  /**< copyright string */
    KiSStringView m_comment;    /**< optional comment regarding legal use */
} KiSLegalInformation;

/**
 */
KI_NATIVE typedef struct KiSProjectInformation {
    KiTSize       m_structSize;
    KiSStringView m_name;
    KiSVersion    m_version;
    KiSStringView m_desc;
    KiSStringView m_homepage;
} KiSProjectInformation;

/**
 * \struct KiSBuildInformation
 * \brief  records information of the build environment at the time of building
 */
KI_NATIVE typedef struct KiSBuildInformation {
    KiTSize               m_structSize;         /**< size of this structure, in bytes */
    KiSProjectInformation m_projInfo;           /**< name of the project this module is part of (i.e., was originally compiled for) */
    KiSStringView         m_buildConfigId;      /**< build configuration identifier ("debug", "deploy", etc.) */
    KiSStringView         m_compilerId;         /**< name of the compiler used */
    KiSStringView         m_compilerVer;        /**< version of the compiler used to build the target */
    KiSStringView         m_buildDate;          /**< date of building */
    KiSStringView         m_buildTime;          /**< time of building */
    KiSStringView         m_buildSystemId;      /**< build system identifier if applicable */
    KiSStringView         m_buildSystemVer;     /**< version of the build system used if applicable */
    KiSStringView         m_buildSystemGen;     /**< name of the build generator (e.g., Ninja, ...) if applicable */
    KiSStringView         m_buildFlags;         /**< compiler flags used to build the target */
    KiSStringView         m_archId;             /**< architecture ID (e.g., x64-86, armv7, ...) */
    KiTUint64             m_bitness;            /**< bitness of the architecture, i.e., 32 or 64 */
    KiEEndianness         m_endianness;         /**< endianness of the target platform */
    KiSStringView         m_stdVer;             /**< version of the C/C++ standard */
    KiSStringView         m_hostPlatform;       /**< platform the target was built \e on */
    KiSStringView         m_hostPlatformVer;    /**< string representing the host platform's version */
    KiSStringView         m_targetPlatform;     /**< platform the target was built \e for */
    KiSStringView         m_targetPlatformVer;  /**< string representing the target platform's version */
    KiSStringView         m_crtVer;             /**< string representing the version of the C runtime library */
    KiSStringView         m_rootDir;            /**< root directory of the target */
    KiSStringView         m_gitCommitHashLong;  /**< latest git commit (long) if applicable */
    KiSStringView         m_gitCommitHashShort; /**< latest git commit (short) if applicable */
    KiSStringView         m_gitBranchName;      /**< current git branch if applicable */
    KiTByte               m_gitIsDirty;         /**< git dirty flag if applicable */
} KiSBuildInformation;

/**
 * \struct KiSResolvedRequirement
 * \brief  represents a resolved dependency which can be used to feed into injected factory instances
 */
KI_NATIVE typedef struct KiSResolvedRequirement {
    KiTSize        m_structSize; /**< size of this structure, in bytes */
    KiSUuid const *mp_iid;       /**< canonical name of the interface */
    KiSUuid const *mp_cid;       /**< canonical name of the component */
} KiSResolvedRequirement;

/**
 * \struct KiSProvidedInterface
 * \brief  represents an entry for a provided interface (<tt>KI_IMPLEMENTS(...)</tt>) used by \c KiSComponentMetadata
 */
KI_NATIVE typedef struct KiSProvidedInterface {
    KiTSize                   m_structSize;     /**< size of this structure, in bytes */
    KiSStringView             m_ifaceName;      /**< qualified name of the provided interface */
    KiEProvidedInterfaceFlags m_provIfaceFlags; /**< flags specific to this provided instance of the interface */
} KiSProvidedInterface;

/**
 * \struct KiSRequiredInterface
 * \brief  represents an entry for an interface requirement (<tt>KI_REQUIRES(...)</tt>) used by \c KiSComponentMetadata
 */
KI_NATIVE typedef struct KiSRequiredInterface {
    KiTSize                   m_structSize;    /**< size of this struct, in bytes */
    KiSStringView             m_ifaceName;     /**< qualified name of the required interface */
    KiERequiredInterfaceFlags m_reqIfaceFlags; /**< flags specific to this interface requirement */
} KiSRequiredInterface;

/**
 */
KI_NATIVE typedef struct KiSInterfaceMethodParameter {
    KiTSize       m_structSize;
    KiTIndex      m_idx;
    KiTBool       m_isPointer;
    KiTSize       m_paramSize;
    KiTSize       m_paramTypeSize;
    KiSStringView m_type;
    KiSStringView m_ident;
} KiSInterfaceMethodParameter;

/**
 */
KI_NATIVE typedef struct KiSInterfaceMethod {
    KiTSize                      m_structSize;
    KiSStringView                m_retType;
    KiSStringView                m_name;
    KiSInterfaceMethodParameter *mp_params;
    KiTSize                      m_nParams;
} KiSInterfaceMethod;

/**
 * \struct KiSModuleMetadata
 * \brief  represents static metadata for an KiOM module
 */
KI_NATIVE typedef struct KiSModuleMetadata {
    KiTSize             m_structSize;       /**< size of this structure, in bytes */
    KiSUuid             m_uuid;             /**< universally unique identifier (UUID) of the module */
    KiSStringView       m_name;             /**< canonical name of the module, e.g., \c random */
    KiSStringView       m_shortName;        /**< short name of the module, e.g., \c RND */
    KiSStringView       m_brief;            /**< brief module description */
    KiSStringView       m_docs;             /**< more detailed module documentation */
    KiSVersion          m_version;          /**< version of the module */
    KiSLegalInformation m_legalInfo;        /**< structure containing legal information regarding this module */
    KiEModuleFlags      m_flags;            /**< additional module flags */
    KiTSize             m_nInterfaces;      /**< number of interfaces defined by the module */
    KiTSize             m_nComponents;      /**< number of components exposed in the module */
    KiTSize             m_nTestSuites;      /**< number of test suites contained in the module */
    KiTSize             m_nStaticResources; /**< number of static resources present in the module */
    KiTSize             m_nErrStringifiers; /**< number of error stringifiers present in the module */
    KiSStringView       m_extraMetadata;    /**< extra metadata, as JSON object */
    KiSStringView       m_fullMetadata;     /**< full metadata, as JSON object */
} KiSModuleMetadata;

/**
 * \struct KiSInterfaceMetadata
 * \brief  represents the static metadata for an interface
 */
KI_NATIVE typedef struct KiSInterfaceMetadata {
    KiTSize              m_structSize;    /**< size of this structure, in bytes */
    KiSUuid              m_uuid;          /**< interface UUID */
    KiSStringView        m_name;          /**< \e canonical interface name (i.e., what's put into <tt>KI_INTERFACE(...)</tt>) */
    KiSStringView        m_displayName;   /**< opt. name for display purposes */
    KiSStringView        m_shortName;     /**< opt. short identifier */
    KiSStringView        m_brief;         /**< brief description */
    KiSStringView        m_docs;          /**< more detailed description; supports HTML and Markup */
    KiSVersion           m_version;       /**< interface version */
    KiSLegalInformation  m_legalInfo;     /**< structure containing legal information regarding the interface */
    KiSStringView       *mp_baseIfaces;   /**< names of the direct base interfaces */
    KiTSize              m_nBaseIfaces;   /**< number of elements in \c mp_baseIfaces */
    KiSInterfaceMethod  *mp_methods;      /**< methods directly exposed by this interface */
    KiTSize              m_nMethods;      /**< number of elements in \c mp_methods */
    KiEInterfaceFlags    m_flags;         /**< flags specific to this interface */
    KiSStringView        m_extraMetadata; /**< extra metadata, as JSON object */
    KiSStringView        m_fullMetadata;  /**< full metadata, as JSON object */
} KiSInterfaceMetadata;

/**
 * \struct KiSComponentMetadata
 * \brief  represents static metadata for an KiOM component
 */
KI_NATIVE typedef struct KiSComponentMetadata {
    KiTSize               m_structSize;    /**< size of this structure, in bytes */
    KiSUuid               m_compUuid;      /**< universally unique identifier (UUID) of the component */
    KiSStringView         m_canonicalName; /**< canonical name of the KiOM component, e.g., \c KiCModuleLoader */
    KiSStringView         m_displayName;   /**< display name of the KiOM component, e.g., \c ModuleLoader */
    KiSStringView         m_shortName;     /**< short name of the KiOM component, e.g., \c MDL */
    KiEComponentFlags     m_compFlags;     /**< additional component flags */
    KiSStringView         m_brief;         /**< brief component description */
    KiSStringView         m_docs;          /**< more detailed component documentation */
    KiSVersion            m_version;       /**< version of the KiOM component */
    KiSProvidedInterface *mp_provIfaces;   /**< interfaces provided by this component */
    KiTSize               m_nProvIfaces;   /**< number of entries in \c mp_provIfaces */
    KiSRequiredInterface *mp_reqIfaces;    /**< interfaces required by this component */
    KiTSize               m_nReqIfaces;    /**< number of entries in \c m_reqIfacesCnt */
    KiSLegalInformation   m_legalInfo;     /**< structure containing legal information regarding this component */
    KiSStringView         m_extraMetadata; /**< extra metadata, as JSON object */
    KiSStringView         m_fullMetadata;  /**< full metadata, as JSON object */
} KiSComponentMetadata;

/**
 * \struct KiSComponentControlContext
 * \brief  exposes global KiOM components to a component that is being created or destroyed
 */
KI_NATIVE typedef struct KiSComponentControlContext {
    KiTSize                     m_structSize;  /**< size of this structure, in bytes */
    KiEInstanceScope            m_instScope;   /**< creation context, or \e scope, of the current instance */
    KiIModule                  *mp_modDesc;    /**< reference to the instance of the module the current component resides in */
    KiIBase                    *mp_thisInst;   /**< reference to the instance of the current component */
    KiSModuleMetadata    const *mp_modMtdPtr;  /**< pointer to the instance of the metadata of the current component's module */
    KiSComponentMetadata const *mp_compMtdPtr; /**< pointer to the instance of the metadata of the current component */
    KiTVoid                    *mp_extraParam; /**< extra context; can be \c nullptr */
} KiSComponentControlContext;

/**
 */
KI_NATIVE typedef struct KiSMetaComponent {
    KiTSize     m_structSize; /**< size of this structure, in bytes */
    KiTRefCount m_refCount;   /**< internal reference count */
} KiSMetaComponent;


/**
 * \interface KiIBase
 * \brief     represents the most fundamental interface that any Kira component must implement in order to guarantee
 *            consistency across the entire architecture
 * 
 * The \c KiIBase interface implements the most basic functions required by Kira's object model. It covers nothing
 * but the basic lifetime management functionality. All objects must directly or indirectly derive from this interface.
 * However, there is no hard requirement as to whether or not the component actually implements reference-counting.
 */
KI_INTERFACE(KiIBase) KI_AUXILIARY KI_BUILTIN {
    KI_METADATA(
        "uuid":    "af8192f6-d8fc-4522-947a-a143b6361c81",
        "name":    "KiIBase",
        "dname":   "IBase",
        "sname":   "BASE",
        "brief":   "base interface containing methods for capability querying and lifetime management",
        "version": [1, 0, 0],
        "legal":   {
            "author":    "TophUwO",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2025 TophUwO"
        }
    )

    /**
     * \brief   allows the user of the component to query what interfaces the current instance provides
     * 
     * In general, components and their instances can implement multiple interfaces. The behavior of this function must
     * reflect that by never failing when provided with an interface ID of which the interface is implemented by the
     * current instance.
     * 
     * \param   [in,out] self pointer to the current \c KiIBase instance
     * \param   [in] iidPtr pointer to the interface identifier of the interface that is
     *               to be queried
     * \param   [in] resPtr pointer to a variable that will receive the pointer to the
     *               queried interface
     * \return  \c KiErr_Ok on success, non-zero on failure
     * \note    \li If the function succeeds, the function returns \c KiErr_Ok and \c *resPtr will be initialized to a
     *              non-<tt>NULL</tt> value that points to the queried interface VTable.
     *          \li If the function fails, the function returns non-zero and \c *resPtr will be initialized with
     *              <tt>NULL</tt>.
     *          \li The function must return \c KiErr_Ok for all interfaces in the derivation tree. That is, if the
     *              component implements an interface \c KiIList which derives from two interfaces called <tt>KiICollection</tt>
     *              and <tt>KiIOrdered</tt>, which both derive directly from <tt>KiIBase</tt>, then <tt>QueryInterface()</tt>
     *              must succeed for all queries that query one of the aforementioned interfaces.
     *          \li The behavior when queried for interfaces must be fixed. This means that if the first query for an
     *              interface of a special type (expressed through interface name) succeeds, all subsequent queries for
     *              the same interface must also succeed. Similarly, if it fails, all subsequent queries for the same
     *              interface must fail.
     *          \li If this function succeeds, it must call the queried interface's <tt>Acquire()</tt> method.
     * \note    Aside from the basic requirements outlined above, <tt>QueryInterface()</tt> also needs to satisfy the
     *          requirements of an <em>equivalency relation</em>. In essence,
     *           <ol>
     *            <li>
     *             It must be <em>reflexive</em>. This means that if you query an object for interface an <tt>A</tt>,
     *             this succeeds, and use the queried interface to query for <tt>A</tt> again, then the second query 
     *             must succeed as well.
     *            </li>
     *            <li>
     *             It must be <em>symmetric</em>. This means if you query an object for an interface <tt>B</tt> using
     *             the object's <tt>A</tt>, this query succeeds, and you use the queried <tt>B</tt> interface to query
     *             for <tt>A</tt>, then this second query must succeed, too.
     *            </li>
     *            <li>
     *             It must be <em>transitive</em>. That is, if you successfully query for an interface <tt>A</tt>, use
     *             <tt>A</tt> to successfully query for <tt>B</tt>, and use <tt>B</tt> to successfully query for
     *             <tt>C</tt>, then using <tt>A</tt> to query for <tt>C</tt> must also succeed.
     *            </li>
     *           </ol>
     * \warning Passing invalid parameters, that is, \c NULL for any of the pointer parameters generally invokes
     *          undefined behavior unless otherwise specified in the documentation for a concrete interface.
     */
    KiEErrorCode (KI_CALL *QueryInterface)(KiIBase *self, KiTChar const *iId, KiTVoid **resPtr);
    /**
     * \brief   increments the reference count of the current instance
     * 
     * The lifetime of KiOM objects is controlled via reference-counting. The reference count is a numeric value that
     * holds the number of outstanding references to the current object. If this value is greater than zero, this means
     * we have at least one piece of code that still holds a reference to the current instance; we cannot destroy the
     * object in such a case. If the reference count reaches zero in response to a call to <tt>Release()</tt>, the
     * object can be safely destroyed.
     * 
     * \param   [in,out] self pointer to the current \c KiIBase instance
     * \return  the new (incremented) reference count
     * 
     * \par Remarks
     *   \c KiIBase::Acquire() and \c KiIBase::Release() are required to return accurate values. This means, for every
     *   call to \c KiIBase::Acquire(), the reference count must be incremented by exactly one. For every call to
     *   \c KiIBase::Release(), the reference count must be decremented by exactly one. The function then has to return
     *   the new reference count.
     */
    KiTInt32 (KI_CALL *Acquire)(KiIBase *self);
    /**
     * \brief   decrements the reference count of the current instance
     * 
     * The lifetime of KiOM objects is controlled via reference-counting. The reference count is a numeric value that
     * holds the number of outstanding references to the current object. If this value is greater than zero, this means
     * we have at least one piece of code that still holds a reference to the current instance; we cannot destroy the
     * object in such a case. If the reference count reaches zero in response to a call to <tt>Release()</tt>, the
     * object can be safely destroyed.
     * 
     * \param   [in,out] self pointer to the current \c KiIBase instance
     * \return  the new (decremented) reference count
     * 
     * \par Remarks
     *   \c KiIBase::Acquire() and \c KiIBase::Release() are required to return accurate values. This means, for every
     *   call to \c KiIBase::Acquire(), the reference count must be incremented by exactly one. For every call to
     *   \c KiIBase::Release(), the reference count must be decremented by exactly one. The function then has to return
     *   the new reference count.
     */
    KiTInt32 (KI_CALL *Release)(KiIBase *self);

    /**
     * \brief   queries the <em>meta component</em> of the current instance
     * 
     * The meta-component is a set of data fields contained within every KiOM component. The meta-component is essential
     * to the following standard Kira features:
     * <ul>
     *  <li>reference-counting</li>
     * </ul>
     * This method, like all methods in \c KiIBase, is normally automatically-generated by the Kira pre-processor
     * (\c KiPP.)
     *
     * \param   [in,out] self pointer to the \c KiIBase interface of the current instance
     * \return  pointer to the meta-component
     * \warning Calling this function does not increment the reference-count of the current instance, meaning the
     *          pointer returned by this method may go invalid unexpectedly. If you need to cache the pointer, add a
     *          reference before calling this method.
     */
    KiSMetaComponent *(KI_CALL *GetMetaComponent)(KiIBase *self);
};

/**
 */
KI_INTERFACE(KiIComponentFactory) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    KI_METADATA(
        "uuid":    "e31cbb9c-1160-4587-b23f-f394b23b889a",
        "name":    "KiIComponentFactory",
        "dname":   "IComponentFactory",
        "sname":   "COMPFAC",
        "brief":   "interface exposing an API that is able to create components",
        "version": [1, 0, 0],
        "legal":   {
            "author":    "TophUwO",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2025 TophUwO"
        }
    )
    KI_BASE(KiIComponentFactory)

    /**
     */
    KiSComponentMetadata const *(KI_CALL *GetComponentMetadataList)(KiIComponentFactory *self, KiTSize *compCntPtr);

    /**
     */
    KiEErrorCode (KI_CALL *CreateComponent)(
        KiIComponentFactory *self,
        KiSUuid const *cid,
        KiSUuid const *iid,
        KiTVoid *extraParam,
        KiIBase **resPtr
    );
};

/**
 */
KI_INTERFACE(KiIModule) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    KI_METADATA(
        "uuid":    "67c3c075-99f5-45ed-b127-081eb29c1e4a",
        "name":    "KiIModule",
        "dname":   "IModule",
        "sname":   "MODULE",
        "brief":   "interface exposing information for the runtime reflection system",
        "version": [1, 0, 0],
        "legal":   {
            "author":    "TophUwO",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2024-2025 TophUwO"
        }
    )
    KI_BASE(KiIModule)

    /**
     */
    KiSBuildInformation const *(KI_CALL *GetBuildInformation)(KiIModule *self);
    /**
     */
    KiSInterfaceMetadata const *(KI_CALL *GetInterfaceMetadataList)(KiIModule *self, KiTSize *ifaceCntPtr);
    /**
     */
    KiSComponentMetadata const *(KI_CALL *GetComponentMetadataList)(KiIModule *self, KiTSize *compCntPtr);
    /**
     */
    KiSModuleMetadata const *(KI_CALL *GetModuleMetadata)(KiIModule *self);

    /**
     */
    KiEErrorCode (KI_CALL *QueryComponentFactory)(KiIModule *self, KiIComponentFactory **resPtr);

    /**
     */
    KiEErrorCode (KI_CALL *Prologue)(KiIModule *self, KiTVoid *extraParam);
    /**
     */
    KiEErrorCode (KI_CALL *Epilogue)(KiIModule *self);
};

/**
 * \interface KiIErrorStringifier
 * \brief     the interface actually used for stringification of numeric error codes
 * 
 * \par Introduction
 *   Since Kira is a completely modular game engine and modules provide different error conditions, they might also
 *   introduce new error codes. To allow per-module error codes, a module might expose one or more objects that
 *   implement the \c KiIErrorStringifier interface. This interface provides error code details to a global singleton
 *   that implements the \c KiIErrorStringificationService. Everytime this object is queried for an error code
 *   stringification, it will resolve the error code and invoke the corresponding \c KiIErrorStringifier instance.
 * \par Scopes
 *   Since modules in Kira are not cooperative, there might be overlap between error code ranges provided by their
 *   respective \c KiIErrorStringifiers. To resolve them, when an \c KiIErrorStringifier registers itself with the
 *   \c KiIErrorStringificationService, the service negotiates an <em>error code offset</em>, that is, a numeric value
 *   that maps <em>module scope</em> to <em>global scope</em> and vice versa. Modules provide the error code range and
 *   the service will return the error code offset. Modules now use this value to return global error codes, simply by
 *   adding the offset to the module error code. When the error code is resolve, it is translated back into the module
 *   error code by subtracting the offset again. 
 */
KI_INTERFACE(KiIErrorStringifier) KI_EXTENDS(KiIBase) KI_AUXILIARY KI_BUILTIN {
    KI_METADATA(
        "uuid":    "3328e548-4c67-47e9-ab2b-18968ec02c6b",
        "name":    "KiIErrorStringifier",
        "dname":   "IErrorStringifier",
        "sname":   "ERRSTR",
        "brief":   "interface providing stringifications for module-specific error codes",
        "version": [1, 0, 0, 0]
        "legal":   {
            "author":    "TophUwO",
            "contact":   "tophuwo01@gmail.com",
            "license":   "Apache License 2.0",
            "copyright": "(c) 2025 TophUwO"
        }
    )
    KI_BASE(KiIErrorStringifier)

    /**
     * \brief  retrieves the stringification of the provided numeric error code
     * \param  [in,out] self pointer to the current \c KiIErrorStringifier instance
     * \param  [in] errorCode numeric error code of which the string representation is to be retrieved
     * \return pointer to the \c KiSStringView corresponding to the error representation of the given numeric error code,
     *         or the string representation of \c KiErr_Unknown if the error code representation cannot be retrieved
     * \note   No implementation should ever return \c NULL.
     */
    KiSStringView const *(KI_CALL *QueryErrorString)(KiIErrorStringifier *self, KiEErrorCode errorCode);
    /**
     * \brief
     */
    KiSStringView const *(KI_CALL *QueryErrorBrief)(KiIErrorStringifier *self, KiEErrorCode errorCode);
    /**
     * \brief
     */
    KiSStringView const *(KI_CALL *QueryErrorDetails)(KiIErrorStringifier *self, KiEErrorCode errorCode);
};


