#pragma once

typedef char16_t Il2CppChar;
typedef size_t il2cpp_array_size_t;
typedef int32_t TypeDefinitionIndex;
typedef int32_t GenericParameterIndex;
typedef char Il2CppNativeChar;

typedef struct Il2CppMemoryCallbacks Il2CppMemoryCallbacks;
typedef struct Il2CppImage Il2CppImage;
typedef struct Il2CppClass Il2CppClass;
typedef struct Il2CppArrayBounds Il2CppArrayBounds;
typedef struct Il2CppAssembly Il2CppAssembly;
typedef struct Il2CppArrayType Il2CppArrayType;
typedef struct Il2CppGenericClass Il2CppGenericClass;
typedef struct Il2CppReflectionType Il2CppReflectionType;
typedef struct MonitorData MonitorData;
typedef Il2CppClass Il2CppVTable;
typedef struct Il2CppDomain Il2CppDomain;
typedef struct Il2CppException Il2CppException;
typedef struct Il2CppObject Il2CppObject;
typedef struct Il2CppReflectionMethod Il2CppReflectionMethod;
typedef struct Il2CppString Il2CppString;
typedef struct Il2CppThread Il2CppThread;
typedef struct Il2CppStackFrameInfo Il2CppStackFrameInfo;
typedef struct Il2CppManagedMemorySnapshot Il2CppManagedMemorySnapshot;
typedef struct Il2CppDebuggerTransport Il2CppDebuggerTransport;
typedef struct Il2CppMethodDebugInfo Il2CppMethodDebugInfo;
typedef struct Il2CppCustomAttrInfo Il2CppCustomAttrInfo;
typedef const struct ___Il2CppMetadataTypeHandle *Il2CppMetadataTypeHandle [[maybe_unused]];
typedef const struct ___Il2CppMetadataGenericParameterHandle *Il2CppMetadataGenericParameterHandle [[maybe_unused]];

typedef void (*Il2CppMethodPointer)();

typedef void (*il2cpp_register_object_callback)(Il2CppObject **arr, int size, void *userdata);

typedef void *(*il2cpp_liveness_reallocate_callback)(void *ptr, size_t size, void *userdata);

typedef void (*Il2CppFrameWalkFunc)(const Il2CppStackFrameInfo *info, void *user_data);

typedef size_t(*Il2CppBacktraceFunc)(Il2CppMethodPointer *buffer, size_t maxSize);

typedef const Il2CppNativeChar *(*Il2CppSetFindPlugInCallback)(const Il2CppNativeChar *);

typedef void (*Il2CppLogCallback)(const char *);

typedef enum {
    IL2CPP_UNHANDLED_POLICY_LEGACY [[maybe_unused]],
    IL2CPP_UNHANDLED_POLICY_CURRENT [[maybe_unused]]
} Il2CppRuntimeUnhandledExceptionPolicy;

typedef enum {
    IL2CPP_GC_MODE_DISABLED [[maybe_unused]] = 0,
    IL2CPP_GC_MODE_ENABLED [[maybe_unused]] = 1,
    IL2CPP_GC_MODE_MANUAL [[maybe_unused]] = 2
} Il2CppGCMode;

typedef enum Il2CppStat {
    IL2CPP_STAT_NEW_OBJECT_COUNT [[maybe_unused]],
    IL2CPP_STAT_INITIALIZED_CLASS_COUNT [[maybe_unused]],
    IL2CPP_STAT_METHOD_COUNT [[maybe_unused]],
    IL2CPP_STAT_CLASS_STATIC_DATA_SIZE [[maybe_unused]],
    IL2CPP_STAT_GENERIC_INSTANCE_COUNT [[maybe_unused]],
    IL2CPP_STAT_GENERIC_CLASS_COUNT [[maybe_unused]],
    IL2CPP_STAT_INFLATED_METHOD_COUNT [[maybe_unused]],
    IL2CPP_STAT_INFLATED_TYPE_COUNT [[maybe_unused]],
} Il2CppStat;

typedef enum Il2CppTypeEnum {
    IL2CPP_TYPE_END [[maybe_unused]] = 0x00,
    IL2CPP_TYPE_VOID [[maybe_unused]] = 0x01,
    IL2CPP_TYPE_BOOLEAN [[maybe_unused]] = 0x02,
    IL2CPP_TYPE_CHAR [[maybe_unused]] = 0x03,
    IL2CPP_TYPE_I1 [[maybe_unused]] = 0x04,
    IL2CPP_TYPE_U1 [[maybe_unused]] = 0x05,
    IL2CPP_TYPE_I2 [[maybe_unused]] = 0x06,
    IL2CPP_TYPE_U2 [[maybe_unused]] = 0x07,
    IL2CPP_TYPE_I4 [[maybe_unused]] = 0x08,
    IL2CPP_TYPE_U4 [[maybe_unused]] = 0x09,
    IL2CPP_TYPE_I8 [[maybe_unused]] = 0x0a,
    IL2CPP_TYPE_U8 [[maybe_unused]] = 0x0b,
    IL2CPP_TYPE_R4 [[maybe_unused]] = 0x0c,
    IL2CPP_TYPE_R8 [[maybe_unused]] = 0x0d,
    IL2CPP_TYPE_STRING [[maybe_unused]] = 0x0e,
    IL2CPP_TYPE_PTR [[maybe_unused]] = 0x0f,
    IL2CPP_TYPE_BYREF [[maybe_unused]] = 0x10,
    IL2CPP_TYPE_VALUETYPE [[maybe_unused]] = 0x11,
    IL2CPP_TYPE_CLASS [[maybe_unused]] = 0x12,
    IL2CPP_TYPE_VAR [[maybe_unused]] = 0x13,
    IL2CPP_TYPE_ARRAY [[maybe_unused]] = 0x14,
    IL2CPP_TYPE_GENERICINST [[maybe_unused]] = 0x15,
    IL2CPP_TYPE_TYPEDBYREF [[maybe_unused]] = 0x16,
    IL2CPP_TYPE_I [[maybe_unused]] = 0x18,
    IL2CPP_TYPE_U [[maybe_unused]] = 0x19,
    IL2CPP_TYPE_FNPTR [[maybe_unused]] = 0x1b,
    IL2CPP_TYPE_OBJECT [[maybe_unused]] = 0x1c,
    IL2CPP_TYPE_SZARRAY [[maybe_unused]] = 0x1d,
    IL2CPP_TYPE_MVAR [[maybe_unused]] = 0x1e,
    IL2CPP_TYPE_CMOD_REQD [[maybe_unused]] = 0x1f,
    IL2CPP_TYPE_CMOD_OPT [[maybe_unused]] = 0x20,
    IL2CPP_TYPE_INTERNAL [[maybe_unused]] = 0x21,
    IL2CPP_TYPE_MODIFIER [[maybe_unused]] = 0x40,
    IL2CPP_TYPE_SENTINEL [[maybe_unused]] = 0x41,
    IL2CPP_TYPE_PINNED [[maybe_unused]] = 0x45,
    IL2CPP_TYPE_ENUM [[maybe_unused]] = 0x55,
    IL2CPP_TYPE_IL2CPP_TYPE_INDEX [[maybe_unused]] = 0xff
} Il2CppTypeEnum;

typedef struct Il2CppType {
    union {
        void *dummy;
        TypeDefinitionIndex klassIndex;
        const Il2CppType *type;
        Il2CppArrayType *array;
        GenericParameterIndex genericParameterIndex;
        Il2CppGenericClass *generic_class;
    } data;
    unsigned int attrs: 16;
    Il2CppTypeEnum type: 8;
    unsigned int num_mods [[maybe_unused]]: 6;
    unsigned int byref [[maybe_unused]]: 1;
    unsigned int pinned [[maybe_unused]]: 1;
} Il2CppType;

typedef struct FieldInfo {
    const char *name;
    const Il2CppType *type;
    Il2CppClass *parent;
    int32_t offset; // If offset is -1, then it's thread static
    uint32_t token;
} FieldInfo;

typedef struct ParameterInfo {
    const char *name;
    int32_t position;
    uint32_t token;
    const Il2CppType *parameter_type [[maybe_unused]];
} ParameterInfo;

typedef struct MethodInfo {
    Il2CppMethodPointer methodPointer [[maybe_unused]];
    uintptr_t invoker_method [[maybe_unused]];
    const char *name [[maybe_unused]];
    Il2CppClass *klass [[maybe_unused]];
    const Il2CppType *return_type [[maybe_unused]];
    const ParameterInfo *parameters [[maybe_unused]];
    uintptr_t methodDefinition [[maybe_unused]];
    uintptr_t genericContainer [[maybe_unused]];
    uint32_t token [[maybe_unused]];
    uint16_t flags [[maybe_unused]];
    uint16_t iflags [[maybe_unused]];
    uint16_t slot [[maybe_unused]];
    uint8_t parameters_count [[maybe_unused]];
    uint8_t is_generic [[maybe_unused]]: 1;
    uint8_t is_inflated [[maybe_unused]]: 1;
    uint8_t wrapper_type [[maybe_unused]]: 1;
    uint8_t is_marshaled_from_native [[maybe_unused]]: 1;
} MethodInfo;

typedef struct Il2CppClass {
    // The following fields are always valid for a Il2CppClass structure
    const void *image;
    void *gc_desc;
    const char *name;
    const char *namespaze;
    Il2CppType byval_arg;
    Il2CppType this_arg;
    Il2CppClass *element_class;
    Il2CppClass *castClass;
    Il2CppClass *declaringType;
    Il2CppClass *parent;
    void *generic_class;
    void *typeMetadataHandle; // non-NULL for Il2CppClass's constructed from type defintions
    const void *interopData;
    Il2CppClass *klass; // hack to pretend we are a MonoVTable. Points to ourself
    // End always valid fields

    // The following fields need initialized before access. This can be done per field or as an aggregate via a call to Class::Init
    FieldInfo *fields; // Initialized in SetupFields
    const void *events; // Initialized in SetupEvents
    const void *properties; // Initialized in SetupProperties
    const MethodInfo **methods; // Initialized in SetupMethods
    Il2CppClass **nestedTypes; // Initialized in SetupNestedTypes
    Il2CppClass **implementedInterfaces; // Initialized in SetupInterfaces
    void *interfaceOffsets; // Initialized in Init
    void *static_fields; // Initialized in Init
    const void *rgctx_data; // Initialized in Init
    // used for fast parent checks
    Il2CppClass **typeHierarchy; // Initialized in SetupTypeHierachy
    // End initialization required fields

    void *unity_user_data;

    uint32_t initializationExceptionGCHandle;

    uint32_t cctor_started;
    uint32_t cctor_finished;
    size_t cctor_thread;

    // Remaining fields are always valid except where noted
    void *genericContainerHandle;
    uint32_t instance_size; // valid when size_inited is true
    uint32_t actualSize;
    uint32_t element_size;
    int32_t native_size;
    uint32_t static_fields_size;
    uint32_t thread_static_fields_size;
    int32_t thread_static_fields_offset;
    uint32_t flags;
    uint32_t token;

    uint16_t method_count; // lazily calculated for arrays, i.e. when rank > 0
    uint16_t property_count;
    uint16_t field_count;
    uint16_t event_count;
    uint16_t nested_type_count;
    uint16_t vtable_count; // lazily calculated for arrays, i.e. when rank > 0
    uint16_t interfaces_count;
    uint16_t interface_offsets_count; // lazily calculated for arrays, i.e. when rank > 0

    uint8_t typeHierarchyDepth; // Initialized in SetupTypeHierachy
    uint8_t genericRecursionDepth;
    uint8_t rank;
    uint8_t minimumAlignment; // Alignment of this type
    uint8_t naturalAligment; // Alignment of this type without accounting for packing
    uint8_t packingSize;

    // this is critical for performance of Class::InitFromCodegen. Equals to initialized && !has_initialization_error at all times.
    // Use Class::UpdateInitializedAndNoError to update
    uint8_t initialized_and_no_error: 1;

    uint8_t valuetype: 1;
    uint8_t initialized: 1;
    uint8_t enumtype: 1;
    uint8_t is_generic: 1;
    uint8_t has_references: 1; // valid when size_inited is true
    uint8_t init_pending: 1;
    uint8_t size_init_pending: 1;
    uint8_t size_inited: 1;
    uint8_t has_finalize: 1;
    uint8_t has_cctor: 1;
    uint8_t is_blittable: 1;
    uint8_t is_import_or_windows_runtime: 1;
    uint8_t is_vtable_initialized: 1;
    uint8_t has_initialization_error: 1;
    void *vtable[0];
} Il2CppClass;

typedef struct EventInfo {
    const char *name;
    const Il2CppType *eventType;
    Il2CppClass *parent;
    const MethodInfo *add;
    const MethodInfo *remove;
    const MethodInfo *raise;
    uint32_t token;
} EventInfo;

typedef struct PropertyInfo {
    Il2CppClass *parent;
    const char *name;
    const MethodInfo *get;
    const MethodInfo *set;
    uint32_t attrs;
    uint32_t token;
} PropertyInfo;

typedef struct Il2CppObject {
    union {
        Il2CppClass *klass [[maybe_unused]];
        Il2CppVTable *vtable [[maybe_unused]];
    };
    MonitorData *monitor [[maybe_unused]];
} Il2CppObject;

#define IL2CPP_ARRAY_MAX_INDEX  (reinterpret_cast<int32_t>(0x7fffffff))
#define IL2CPP_ARRAY_MAX_SIZE (reinterpret_cast<uint32_t>(0xffffffff))

typedef struct Il2CppArray {
    Il2CppObject obj;
    Il2CppArrayBounds *bounds [[maybe_unused]];
    il2cpp_array_size_t max_length [[maybe_unused]];
    void *vector[0];
} Il2CppArray;
static const size_t kIl2CppSizeOfArray = (offsetof(Il2CppArray, vector));

typedef struct Il2CppString {
    Il2CppObject object;
    int32_t length; ///< Length of string *excluding* the trailing null (which is included in 'chars').
    Il2CppChar start_char[0];
} Il2CppString;

typedef void *(*InvokerMethod)(Il2CppMethodPointer, const MethodInfo *, void *, void **);

typedef struct Il2CppDelegate {
    Il2CppObject object;
    /* The compiled code of the target method */
    Il2CppMethodPointer method_ptr;
    /* The invoke code */
    InvokerMethod invoke_impl;
    Il2CppObject *target;
    const MethodInfo *method;

    void *delegate_trampoline;

    intptr_t extraArg;

    /*
     * If non-NULL, this points to a memory location which stores the address of
     * the compiled code of the method, or NULL if it is not yet compiled.
     */
    uint8_t **method_code;
    Il2CppReflectionMethod *method_info;
    Il2CppReflectionMethod *original_method_info;
    Il2CppObject *data;

    bool method_is_virtual;
} Il2CppDelegate;

typedef struct MulticastDelegate : Il2CppDelegate {
    Il2CppArray *delegates;
} MulticastDelegate;

typedef struct Il2CppInternalThread {
    Il2CppObject obj;
    int lock_thread_id;
    void *handle;
    void *native_handle;
    Il2CppArray *cached_culture_info;
    Il2CppChar *name;
    int name_len;
    uint32_t state;
    Il2CppObject *abort_exc;
    int abort_state_handle;
    uint64_t tid;
    intptr_t debugger_thread;
    void **static_data;
    void *runtime_thread_info;
    Il2CppObject *current_appcontext;
    Il2CppObject *root_domain_thread;
    Il2CppArray *_serialized_principal;
    int _serialized_principal_version;
    void *appdomain_refs;
    int32_t interruption_requested;
    void *synch_cs;
    bool threadpool_thread;
    bool thread_interrupt_requested;
    int stack_size;
    uint8_t apartment_state;
    int critical_region_level;
    int managed_id;
    uint32_t small_id;
    void *manage_callback;
    void *interrupt_on_stop;
    intptr_t flags;
    void *thread_pinning_ref;
    void *abort_protected_block_count;
    int32_t priority;
    void *owned_mutexes;
    void *suspended;
    int32_t self_suspended;
    size_t thread_state;
    size_t unused2;
    void *last;
} Il2CppInternalThread;

// System.Threading.Thread
typedef struct Il2CppThread {
    Il2CppObject obj;
    Il2CppInternalThread *internal_thread;
    Il2CppObject *start_obj;
    Il2CppException *pending_exception;
    Il2CppObject *principal;
    int32_t principal_version;
    Il2CppDelegate *delegate;
    Il2CppObject *executionContext;
    bool executionContextBelongsToOuterScope;
} Il2CppThread;

typedef struct Il2CppDefaults {
    const Il2CppImage *corlib [[maybe_unused]];
    Il2CppClass *object_class [[maybe_unused]];
    Il2CppClass *byte_class [[maybe_unused]];
    Il2CppClass *void_class [[maybe_unused]];
    Il2CppClass *boolean_class [[maybe_unused]];
    Il2CppClass *sbyte_class [[maybe_unused]];
    Il2CppClass *int16_class [[maybe_unused]];
    Il2CppClass *uint16_class [[maybe_unused]];
    Il2CppClass *int32_class [[maybe_unused]];
    Il2CppClass *uint32_class [[maybe_unused]];
    Il2CppClass *int_class [[maybe_unused]];
    Il2CppClass *uint_class [[maybe_unused]];
    Il2CppClass *int64_class [[maybe_unused]];
    Il2CppClass *uint64_class [[maybe_unused]];
    Il2CppClass *single_class [[maybe_unused]];
    Il2CppClass *double_class [[maybe_unused]];
    Il2CppClass *char_class [[maybe_unused]];
    Il2CppClass *string_class [[maybe_unused]];
    Il2CppClass *enum_class [[maybe_unused]];
    Il2CppClass *array_class [[maybe_unused]];
    Il2CppClass *delegate_class [[maybe_unused]];
    Il2CppClass *multicastdelegate_class [[maybe_unused]];
    Il2CppClass *asyncresult_class [[maybe_unused]];
    Il2CppClass *manualresetevent_class [[maybe_unused]];
    Il2CppClass *typehandle_class [[maybe_unused]];
    Il2CppClass *fieldhandle_class [[maybe_unused]];
    Il2CppClass *methodhandle_class [[maybe_unused]];
    Il2CppClass *systemtype_class [[maybe_unused]];
    Il2CppClass *monotype_class [[maybe_unused]];
    Il2CppClass *exception_class [[maybe_unused]];
    Il2CppClass *threadabortexception_class [[maybe_unused]];
    Il2CppClass *thread_class [[maybe_unused]];
    Il2CppClass *internal_thread_class [[maybe_unused]];
    /*Il2CppClass *transparent_proxy_class;
    Il2CppClass *real_proxy_class;
    Il2CppClass *mono_method_message_class;*/
    Il2CppClass *appdomain_class [[maybe_unused]];
    Il2CppClass *appdomain_setup_class [[maybe_unused]];
    Il2CppClass *field_info_class [[maybe_unused]];
    Il2CppClass *method_info_class [[maybe_unused]];
    Il2CppClass *property_info_class [[maybe_unused]];
    Il2CppClass *event_info_class [[maybe_unused]];
    Il2CppClass *mono_event_info_class [[maybe_unused]];
    Il2CppClass *stringbuilder_class [[maybe_unused]];
    /*Il2CppClass *math_class;*/
    Il2CppClass *stack_frame_class [[maybe_unused]];
    Il2CppClass *stack_trace_class [[maybe_unused]];
    Il2CppClass *marshal_class [[maybe_unused]];
    /*Il2CppClass *iserializeable_class;
    Il2CppClass *serializationinfo_class;
    Il2CppClass *streamingcontext_class;*/
    Il2CppClass *typed_reference_class [[maybe_unused]];
    /*Il2CppClass *argumenthandle_class;*/
    Il2CppClass *marshalbyrefobject_class [[maybe_unused]];
    /*Il2CppClass *monitor_class;
    Il2CppClass *iremotingtypeinfo_class;
    Il2CppClass *runtimesecurityframe_class;
    Il2CppClass *executioncontext_class;
    Il2CppClass *internals_visible_class;*/
    Il2CppClass *generic_ilist_class [[maybe_unused]];
    Il2CppClass *generic_icollection_class [[maybe_unused]];
    Il2CppClass *generic_ienumerable_class [[maybe_unused]];
    Il2CppClass *generic_ireadonlylist_class [[maybe_unused]];
    Il2CppClass *generic_ireadonlycollection_class [[maybe_unused]];
    Il2CppClass *runtimetype_class [[maybe_unused]];
    Il2CppClass *generic_nullable_class [[maybe_unused]];
    /*Il2CppClass *variant_class;
    Il2CppClass *com_object_class;*/
    Il2CppClass *il2cpp_com_object_class [[maybe_unused]];
    /*Il2CppClass *com_interop_proxy_class;
    Il2CppClass *iunknown_class;
    Il2CppClass *idispatch_class;
    Il2CppClass *safehandle_class;
    Il2CppClass *handleref_class;*/
    Il2CppClass *attribute_class [[maybe_unused]];
    Il2CppClass *customattribute_data_class [[maybe_unused]];
    //Il2CppClass *critical_finalizer_object;
    Il2CppClass *version [[maybe_unused]];
    Il2CppClass *culture_info [[maybe_unused]];
    Il2CppClass *async_call_class [[maybe_unused]];
    Il2CppClass *assembly_class [[maybe_unused]];
    Il2CppClass *mono_assembly_class [[maybe_unused]];
    Il2CppClass *assembly_name_class [[maybe_unused]];
    Il2CppClass *mono_field_class [[maybe_unused]];
    Il2CppClass *mono_method_class [[maybe_unused]];
    Il2CppClass *mono_method_info_class [[maybe_unused]];
    Il2CppClass *mono_property_info_class [[maybe_unused]];
    Il2CppClass *parameter_info_class [[maybe_unused]];
    Il2CppClass *mono_parameter_info_class [[maybe_unused]];
    Il2CppClass *module_class [[maybe_unused]];
    Il2CppClass *pointer_class [[maybe_unused]];
    Il2CppClass *system_exception_class [[maybe_unused]];
    Il2CppClass *argument_exception_class [[maybe_unused]];
    Il2CppClass *wait_handle_class [[maybe_unused]];
    Il2CppClass *safe_handle_class [[maybe_unused]];
    Il2CppClass *sort_key_class [[maybe_unused]];
    Il2CppClass *dbnull_class [[maybe_unused]];
    Il2CppClass *error_wrapper_class [[maybe_unused]];
    Il2CppClass *missing_class [[maybe_unused]];
    Il2CppClass *value_type_class [[maybe_unused]];

    // Stuff used by the mono code
    Il2CppClass *threadpool_wait_callback_class;
    MethodInfo *threadpool_perform_wait_callback_method [[maybe_unused]];
    Il2CppClass *mono_method_message_class [[maybe_unused]];

    // Windows.Foundation.IReference`1<T>
    Il2CppClass *ireference_class [[maybe_unused]];
    // Windows.Foundation.IReferenceArray`1<T>
    Il2CppClass *ireferencearray_class [[maybe_unused]];
    // Windows.Foundation.Collections.IKeyValuePair`2<K, V>
    Il2CppClass *ikey_value_pair_class [[maybe_unused]];
    // System.Collections.Generic.KeyValuePair`2<K, V>
    Il2CppClass *key_value_pair_class [[maybe_unused]];
    // Windows.Foundation.Uri
    Il2CppClass *windows_foundation_uri_class [[maybe_unused]];
    // Windows.Foundation.IUriRuntimeClass
    Il2CppClass *windows_foundation_iuri_runtime_class_class [[maybe_unused]];
    // System.Uri
    Il2CppClass *system_uri_class [[maybe_unused]];
    // System.Guid
    Il2CppClass *system_guid_class [[maybe_unused]];

    Il2CppClass *sbyte_shared_enum [[maybe_unused]];
    Il2CppClass *int16_shared_enum [[maybe_unused]];
    Il2CppClass *int32_shared_enum [[maybe_unused]];
    Il2CppClass *int64_shared_enum [[maybe_unused]];

    Il2CppClass *byte_shared_enum [[maybe_unused]];
    Il2CppClass *uint16_shared_enum [[maybe_unused]];
    Il2CppClass *uint32_shared_enum [[maybe_unused]];
    Il2CppClass *uint64_shared_enum [[maybe_unused]];
} Il2CppDefaults;

extern Il2CppDefaults il2cpp_defaults;

struct Int32Object;

typedef struct UInt64 {
    unsigned long m_value;
} UInt64;

typedef struct DateTime {
    UInt64 dateData;
} DateTime;

typedef struct TimeSpan {
} TimeSpan;

typedef struct Double {
    double m_value;
} Double;

typedef struct Int64 {
    long m_value;
} Int64;

typedef struct Boolean {
    bool m_value;
} Boolean;

typedef struct IntPtr {
    void *m_value;
} IntPtr;

// UnityEngine.ScreenOrientation
enum class ScreenOrientation {
    Unknown,
    Portrait,
    PortraitUpsideDown,
    LandscapeLeft,
    LandscapeRight,
    AutoRotation,
    Landscape = 3
};

// UnityEngine.Color
typedef struct Color_t {
public:
    float r, g, b, a;
} Color_t;

// UnityEngine.Vector2
typedef struct Vector2_t {
public:
    float x, y;
} Vector2_t;

// UnityEngine.Vector2Int
struct Vector2Int_t {
public:
    int x, y;
};

// UnityEngine.Vector3
typedef struct Vector3_t {
public:
    float x, y, z;
} Vector3_t [[maybe_unused]];

struct Rect_t {
public:
    short x, y, width, height;
};


struct Quaternion_t {
public:
    float w, x, y, z;
};

// UnityEngine.TextGenerationSettings
typedef struct TextGenerationSettings_t {
public:
    // UnityEngine.Font UnityEngine.TextGenerationSettings::font
    void *font [[maybe_unused]];
    // UnityEngine.Color UnityEngine.TextGenerationSettings::color
    Color_t color [[maybe_unused]];
    // System.Int32 UnityEngine.TextGenerationSettings::fontSize
    int32_t fontSize [[maybe_unused]];
    // System.Single UnityEngine.TextGenerationSettings::lineSpacing
    float lineSpacing [[maybe_unused]];
    // System.Boolean UnityEngine.TextGenerationSettings::richText
    bool richText [[maybe_unused]];
    // System.Single UnityEngine.TextGenerationSettings::scaleFactor
    float scaleFactor [[maybe_unused]];
    // UnityEngine.FontStyle UnityEngine.TextGenerationSettings::fontStyle
    int32_t fontStyle [[maybe_unused]];
    // UnityEngine.TextAnchor UnityEngine.TextGenerationSettings::textAnchor
    int32_t textAnchor [[maybe_unused]];
    // System.Boolean UnityEngine.TextGenerationSettings::alignByGeometry
    bool alignByGeometry [[maybe_unused]];
    // System.Boolean UnityEngine.TextGenerationSettings::resizeTextForBestFit
    bool resizeTextForBestFit [[maybe_unused]];
    // System.Int32 UnityEngine.TextGenerationSettings::resizeTextMinSize
    int32_t resizeTextMinSize [[maybe_unused]];
    // System.Int32 UnityEngine.TextGenerationSettings::resizeTextMaxSize
    int32_t resizeTextMaxSize [[maybe_unused]];
    // System.Boolean UnityEngine.TextGenerationSettings::updateBounds
    bool updateBounds [[maybe_unused]];
    // UnityEngine.VerticalWrapMode UnityEngine.TextGenerationSettings::verticalOverflow
    int32_t verticalOverflow [[maybe_unused]];
    // UnityEngine.HorizontalWrapMode UnityEngine.TextGenerationSettings::horizontalOverflow
    int32_t horizontalOverflow [[maybe_unused]];
    // UnityEngine.Vector2 UnityEngine.TextGenerationSettings::generationExtents
    Vector2_t generationExtents [[maybe_unused]];
    // UnityEngine.Vector2 UnityEngine.TextGenerationSettings::pivot
    Vector2_t pivot [[maybe_unused]];
    // System.Boolean UnityEngine.TextGenerationSettings::generateOutOfBounds
    bool generateOutOfBounds [[maybe_unused]];
} TextGenerationSettings_t [[maybe_unused]];

typedef struct Int32Object {
    int m_value;
} Int32Object;

enum class NotificationStyle {
    None,
    BigTextStyle = 2,
};

enum class GroupAlertBehaviours {
    GroupAlertAll,
    GroupAlertSummary,
    GroupAlertChildren,
};

typedef struct AndroidNotification {
    Il2CppString *Title;
    Il2CppString *Text;
    Il2CppString *SmallIcon;
    DateTime FireTime;
    Il2CppString *LargeIcon;
    NotificationStyle Style;
    int Number;
    Boolean ShouldAutoCancel;
    Boolean UsesStopwatch;
    Il2CppString *Group;
    Boolean GroupSummary;
    GroupAlertBehaviours GroupAlertBehaviour;
    Il2CppString *SortKey;
    Il2CppString *IntentData;
    Boolean ShowTimestamp;
    Boolean ShowCustomTimestamp;
    Color_t m_Color;
    TimeSpan m_RepeatInterval;
    DateTime m_CustomTimestamp;
} AndroidNotification;
