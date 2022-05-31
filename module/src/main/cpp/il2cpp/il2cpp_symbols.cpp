//
// Created by kimji on 2022-05-01.
//
#include "il2cpp_symbols.h"

Il2CppDefaults il2cpp_defaults;


char* il2cpp_array_addr_with_size(Il2CppArray *array, int32_t size, uintptr_t idx)
{
    return ((char*)array) + kIl2CppSizeOfArray + size * idx;
}

namespace il2cpp_symbols {
    Il2CppDomain *il2cpp_domain = nullptr;

    void init(Il2CppDomain *domain) {
        il2cpp_domain = domain;

        memset(&il2cpp_defaults, 0, sizeof(Il2CppDefaults));

#define DEFAULTS_INIT(field, ns, n) do { il2cpp_defaults.field = il2cpp_class_from_name(il2cpp_defaults.corlib, ns, n); } while (0)

#define DEFAULTS_INIT_TYPE(field, ns, n, nativetype) do { DEFAULTS_INIT(field, ns, n); } while (0)

#define DEFAULTS_INIT_OPTIONAL(field, ns, n) do { il2cpp_defaults.field = il2cpp_class_from_name(il2cpp_defaults.corlib, ns, n); } while (0)

#define DEFAULTS_INIT_TYPE_OPTIONAL(field, ns, n, nativetype) do { DEFAULTS_INIT_OPTIONAL(field, ns, n); } while (0)

        il2cpp_defaults.corlib = il2cpp_get_corlib();
        DEFAULTS_INIT(object_class, "System", "Object");
        DEFAULTS_INIT(void_class, "System", "Void");
        DEFAULTS_INIT_TYPE(boolean_class, "System", "Boolean", bool);
        DEFAULTS_INIT_TYPE(byte_class, "System", "Byte", uint8_t);
        DEFAULTS_INIT_TYPE(sbyte_class, "System", "SByte", int8_t);
        DEFAULTS_INIT_TYPE(int16_class, "System", "Int16", int16_t);
        DEFAULTS_INIT_TYPE(uint16_class, "System", "UInt16", uint16_t);
        DEFAULTS_INIT_TYPE(int32_class, "System", "Int32", int32_t);
        DEFAULTS_INIT_TYPE(uint32_class, "System", "UInt32", uint32_t);
        DEFAULTS_INIT(uint_class, "System", "UIntPtr");
        DEFAULTS_INIT_TYPE(int_class, "System", "IntPtr", intptr_t);
        DEFAULTS_INIT_TYPE(int64_class, "System", "Int64", int64_t);
        DEFAULTS_INIT_TYPE(uint64_class, "System", "UInt64", uint64_t);
        DEFAULTS_INIT_TYPE(single_class, "System", "Single", float);
        DEFAULTS_INIT_TYPE(double_class, "System", "Double", double);
        DEFAULTS_INIT_TYPE(char_class, "System", "Char", Il2CppChar);
        DEFAULTS_INIT(string_class, "System", "String");
        DEFAULTS_INIT(enum_class, "System", "Enum");
        DEFAULTS_INIT(array_class, "System", "Array");
#if !IL2CPP_TINY
        DEFAULTS_INIT_TYPE(delegate_class, "System", "Delegate", Il2CppDelegate);
        DEFAULTS_INIT_TYPE(multicastdelegate_class, "System", "MulticastDelegate",
                           Il2CppMulticastDelegate);
        DEFAULTS_INIT(asyncresult_class, "System.Runtime.Remoting.Messaging", "AsyncResult");
        DEFAULTS_INIT_TYPE(async_call_class, "System", "MonoAsyncCall", Il2CppAsyncCall);
        DEFAULTS_INIT(manualresetevent_class, "System.Threading", "ManualResetEvent");
#endif // !IL2CPP_TINY
        //DEFAULTS_INIT(typehandle_class, "System", "RuntimeTypeHandle");
        //DEFAULTS_INIT(methodhandle_class, "System", "RuntimeMethodHandle");
        //DEFAULTS_INIT(fieldhandle_class, "System", "RuntimeFieldHandle");
        DEFAULTS_INIT(systemtype_class, "System", "Type");
#if !IL2CPP_TINY
        DEFAULTS_INIT_TYPE(monotype_class, "System", "MonoType", Il2CppReflectionMonoType);
#endif
        //DEFAULTS_INIT(exception_class, "System", "Exception");
        //DEFAULTS_INIT(threadabortexcepXtion_class, "System.Threading", "ThreadAbortException");
        DEFAULTS_INIT_TYPE(thread_class, "System.Threading", "Thread", Il2CppThread);
        DEFAULTS_INIT_TYPE(internal_thread_class, "System.Threading", "InternalThread",
                           Il2CppInternalThread);
        DEFAULTS_INIT_TYPE(runtimetype_class, "System", "RuntimeType", Il2CppReflectionRuntimeType);
#if !IL2CPP_TINY
        DEFAULTS_INIT(appdomain_class, "System", "AppDomain");
        DEFAULTS_INIT(appdomain_setup_class, "System", "AppDomainSetup");
        DEFAULTS_INIT(field_info_class, "System.Reflection", "FieldInfo");
        DEFAULTS_INIT(method_info_class, "System.Reflection", "MethodInfo");
        DEFAULTS_INIT(property_info_class, "System.Reflection", "PropertyInfo");
        DEFAULTS_INIT_TYPE(event_info_class, "System.Reflection", "EventInfo",
                           Il2CppReflectionEvent);
        DEFAULTS_INIT_TYPE(mono_event_info_class, "System.Reflection", "MonoEventInfo",
                           Il2CppReflectionMonoEventInfo);
        DEFAULTS_INIT_TYPE(stringbuilder_class, "System.Text", "StringBuilder",
                           Il2CppStringBuilder);
        DEFAULTS_INIT_TYPE(stack_frame_class, "System.Diagnostics", "StackFrame", Il2CppStackFrame);
        DEFAULTS_INIT(stack_trace_class, "System.Diagnostics", "StackTrace");
        DEFAULTS_INIT_TYPE(typed_reference_class, "System", "TypedReference", Il2CppTypedRef);
#endif
        DEFAULTS_INIT(generic_ilist_class, "System.Collections.Generic", "IList`1");
        DEFAULTS_INIT(generic_icollection_class, "System.Collections.Generic", "ICollection`1");
        DEFAULTS_INIT(generic_ienumerable_class, "System.Collections.Generic", "IEnumerable`1");
        DEFAULTS_INIT(generic_ireadonlylist_class, "System.Collections.Generic", "IReadOnlyList`1");
        DEFAULTS_INIT(generic_ireadonlycollection_class, "System.Collections.Generic",
                      "IReadOnlyCollection`1");
        DEFAULTS_INIT(generic_nullable_class, "System", "Nullable`1");
#if !IL2CPP_TINY
        DEFAULTS_INIT(version, "System", "Version");
        DEFAULTS_INIT(culture_info, "System.Globalization", "CultureInfo");
        DEFAULTS_INIT_TYPE(assembly_class, "System.Reflection", "Assembly",
                           Il2CppReflectionAssembly);
        DEFAULTS_INIT_TYPE(assembly_name_class, "System.Reflection", "AssemblyName",
                           Il2CppReflectionAssemblyName);
#endif // !IL2CPP_TINY
        DEFAULTS_INIT_TYPE(mono_assembly_class, "System.Reflection", "MonoAssembly",
                           Il2CppReflectionAssembly);
#if !IL2CPP_TINY
        DEFAULTS_INIT_TYPE(mono_field_class, "System.Reflection", "MonoField",
                           Il2CppReflectionField);
        DEFAULTS_INIT_TYPE(mono_method_class, "System.Reflection", "MonoMethod",
                           Il2CppReflectionMethod);
        DEFAULTS_INIT_TYPE(mono_method_info_class, "System.Reflection", "MonoMethodInfo",
                           Il2CppMethodInfo);
        DEFAULTS_INIT_TYPE(mono_property_info_class, "System.Reflection", "MonoPropertyInfo",
                           Il2CppPropertyInfo);
        DEFAULTS_INIT_TYPE(parameter_info_class, "System.Reflection", "ParameterInfo",
                           Il2CppReflectionParameter);
        DEFAULTS_INIT_TYPE(mono_parameter_info_class, "System.Reflection", "MonoParameterInfo",
                           Il2CppReflectionParameter);
        DEFAULTS_INIT_TYPE(module_class, "System.Reflection", "Module", Il2CppReflectionModule);

        DEFAULTS_INIT_TYPE(pointer_class, "System.Reflection", "Pointer", Il2CppReflectionPointer);
        DEFAULTS_INIT_TYPE(exception_class, "System", "Exception", Il2CppException);
        DEFAULTS_INIT_TYPE(system_exception_class, "System", "SystemException",
                           Il2CppSystemException);
        DEFAULTS_INIT_TYPE(argument_exception_class, "System", "ArgumentException",
                           Il2CppArgumentException);
        DEFAULTS_INIT_TYPE(marshalbyrefobject_class, "System", "MarshalByRefObject",
                           Il2CppMarshalByRefObject);
        DEFAULTS_INIT_TYPE(il2cpp_com_object_class, "System", "__Il2CppComObject", Il2CppComObject);
        DEFAULTS_INIT_TYPE(safe_handle_class, "System.Runtime.InteropServices", "SafeHandle",
                           Il2CppSafeHandle);
        DEFAULTS_INIT_TYPE(sort_key_class, "System.Globalization", "SortKey", Il2CppSortKey);
        DEFAULTS_INIT(dbnull_class, "System", "DBNull");
        DEFAULTS_INIT_TYPE_OPTIONAL(error_wrapper_class, "System.Runtime.InteropServices",
                                    "ErrorWrapper", Il2CppErrorWrapper);
        DEFAULTS_INIT(missing_class, "System.Reflection", "Missing");
        DEFAULTS_INIT(attribute_class, "System", "Attribute");
        DEFAULTS_INIT(customattribute_data_class, "System.Reflection", "CustomAttributeData");
        DEFAULTS_INIT(value_type_class, "System", "ValueType");
        DEFAULTS_INIT(key_value_pair_class, "System.Collections.Generic", "KeyValuePair`2");
        DEFAULTS_INIT(system_guid_class, "System", "Guid");
#endif // !IL2CPP_TINY

#if !IL2CPP_TINY
        DEFAULTS_INIT(threadpool_wait_callback_class, "System.Threading",
                      "_ThreadPoolWaitCallback");
        DEFAULTS_INIT(mono_method_message_class, "System.Runtime.Remoting.Messaging",
                      "MonoMethodMessage");

        il2cpp_defaults.threadpool_perform_wait_callback_method = (MethodInfo *) il2cpp_class_get_method_from_name(
                il2cpp_defaults.threadpool_wait_callback_class, "PerformWaitCallback", 0);
#endif

        DEFAULTS_INIT_OPTIONAL(sbyte_shared_enum, "System", "SByteEnum");
        DEFAULTS_INIT_OPTIONAL(int16_shared_enum, "System", "Int16Enum");
        DEFAULTS_INIT_OPTIONAL(int32_shared_enum, "System", "Int32Enum");
        DEFAULTS_INIT_OPTIONAL(int64_shared_enum, "System", "Int64Enum");

        DEFAULTS_INIT_OPTIONAL(byte_shared_enum, "System", "ByteEnum");
        DEFAULTS_INIT_OPTIONAL(uint16_shared_enum, "System", "UInt16Enum");
        DEFAULTS_INIT_OPTIONAL(uint32_shared_enum, "System", "UInt32Enum");
        DEFAULTS_INIT_OPTIONAL(uint64_shared_enum, "System", "UInt64Enum");
    }

    Il2CppClass *get_class(const char *assemblyName, const char *namespaze, const char *klassName) {
        auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
        auto image = il2cpp_assembly_get_image(assembly);
        return il2cpp_class_from_name(image, namespaze, klassName);
    }

    Il2CppMethodPointer get_method_pointer(const char *assemblyName, const char *namespaze,
                                           const char *klassName, const char *name, int argsCount) {
        auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
        auto image = il2cpp_assembly_get_image(assembly);
        auto klass = il2cpp_class_from_name(image, namespaze, klassName);

        return il2cpp_class_get_method_from_name(klass, name, argsCount)->methodPointer;
    }

    const MethodInfo *get_method(const char *assemblyName, const char *namespaze,
                                 const char *klassName, const char *name, int argsCount) {
        auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
        auto image = il2cpp_assembly_get_image(assembly);
        auto klass = il2cpp_class_from_name(image, namespaze, klassName);

        return il2cpp_class_get_method_from_name(klass, name, argsCount);
    }

    Il2CppMethodPointer find_method(const char *assemblyName, const char *namespaze,
                                    const char *klassName,
                                    std::function<bool(const MethodInfo *)> predict) {
        auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
        auto image = il2cpp_assembly_get_image(assembly);
        auto klass = il2cpp_class_from_name(image, namespaze, klassName);

        void *iter = nullptr;
        while (const MethodInfo *method = il2cpp_class_get_methods(klass, &iter)) {
            if (predict(method))
                return method->methodPointer;
        }

        return nullptr;
    }
}
