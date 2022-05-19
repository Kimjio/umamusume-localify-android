#pragma once

typedef uint16_t Il2CppChar;
typedef uintptr_t il2cpp_array_size_t;
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
    unsigned int byref: 1;
    unsigned int pinned [[maybe_unused]]: 1;
} Il2CppType;

typedef struct FieldInfo
{
    const char* name;
    const Il2CppType* type;
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
    uintptr_t klass [[maybe_unused]];
    uintptr_t return_type [[maybe_unused]];
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

typedef struct EventInfo
{
    const char* name;
    const Il2CppType* eventType;
    Il2CppClass* parent;
    const MethodInfo* add;
    const MethodInfo* remove;
    const MethodInfo* raise;
    uint32_t token;
} EventInfo;

typedef struct PropertyInfo
{
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

typedef struct Il2CppArray {
    Il2CppObject obj;
    Il2CppArrayBounds *bounds [[maybe_unused]];
    il2cpp_array_size_t max_length [[maybe_unused]];
    void *vector[32];
} Il2CppArray;

typedef struct Il2CppString {
    Il2CppObject object;
    int32_t length; ///< Length of string *excluding* the trailing null (which is included in 'chars').
    Il2CppChar start_char[0];
} Il2CppString;

// UnityEngine.Color
typedef struct Color_t {
public:
    // System.Single UnityEngine.Color::r
    float r;
    // System.Single UnityEngine.Color::g
    float g;
    // System.Single UnityEngine.Color::b
    float b;
    // System.Single UnityEngine.Color::a
    float a;
} Color_t;

// UnityEngine.Vector2
typedef struct Vector2_t {
public:
    // System.Single UnityEngine.Vector2::x
    float x;
    // System.Single UnityEngine.Vector2::y
    float y;
} Vector2_t;

// UnityEngine.Vector3
typedef struct Vector3_t {
public:
    // System.Single UnityEngine.Vector3::x
    float x;
    // System.Single UnityEngine.Vector3::y
    float y;
    // System.Single UnityEngine.Vector3::z
    float z;
} Vector3_t [[maybe_unused]];

typedef struct Resolution_t
{
public:
    int width;
    int height;
    int herz [[maybe_unused]];
} Resolution_t;

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
