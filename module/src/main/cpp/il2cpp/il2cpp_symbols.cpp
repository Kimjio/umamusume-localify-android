//
// Created by kimji on 2022-05-01.
//
#include "il2cpp_symbols.h"

namespace il2cpp_symbols
{
    Il2CppDomain* il2cpp_domain = nullptr;

    void init(Il2CppDomain* domain)
    {
        il2cpp_domain = domain;
    }

    void* get_class(const char* assemblyName, const char* namespaze, const char* klassName)
    {
        auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
        auto image = il2cpp_assembly_get_image(assembly);
        return il2cpp_class_from_name(image, namespaze, klassName);
    }

    Il2CppMethodPointer get_method_pointer(const char* assemblyName, const char* namespaze,
                                           const char* klassName, const char* name, int argsCount)
    {
        auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
        auto image = il2cpp_assembly_get_image(assembly);
        auto klass = il2cpp_class_from_name(image, namespaze, klassName);

        return il2cpp_class_get_method_from_name(klass, name, argsCount)->methodPointer;
    }

    const MethodInfo* get_method(const char* assemblyName, const char* namespaze,
                                 const char* klassName, const char* name, int argsCount)
    {
        auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
        auto image = il2cpp_assembly_get_image(assembly);
        auto klass = il2cpp_class_from_name(image, namespaze, klassName);

        return il2cpp_class_get_method_from_name(klass, name, argsCount);
    }

    Il2CppMethodPointer find_method(const char* assemblyName, const char* namespaze,
                                    const char* klassName, std::function<bool(const MethodInfo*)> predict)
    {
        auto assembly = il2cpp_domain_assembly_open(il2cpp_domain, assemblyName);
        auto image = il2cpp_assembly_get_image(assembly);
        auto klass = il2cpp_class_from_name(image, namespaze, klassName);

        void* iter = nullptr;
        while (const MethodInfo* method = il2cpp_class_get_methods(klass, &iter))
        {
            if (predict(method))
                return method->methodPointer;
        }

        return nullptr;
    }
}
