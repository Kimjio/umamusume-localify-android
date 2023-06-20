#ifndef UMAMUSUMELOCALIFYANDROID_IL2CPP_SYMBOLS_H
#define UMAMUSUMELOCALIFYANDROID_IL2CPP_SYMBOLS_H

#include "../stdinclude.hpp"
#include "il2cpp-class.h"

#define DO_API(r, n, ...) inline r (*n) (__VA_ARGS__)

#include "il2cpp-api-functions.h"

#undef DO_API

#include "il2cpp-api-functions.hpp"


namespace il2cpp_symbols {
    void init(Il2CppDomain *domain);

    Il2CppClass *get_class(const char *assemblyName, const char *namespaze, const char *klassName);

    Il2CppMethodPointer get_method_pointer(const char *assemblyName, const char *namespaze,
                                           const char *klassName, const char *name, int argsCount);

    template<typename T>
    T get_method_pointer(const char *assemblyName, const char *namespaze,
                         const char *klassName, const char *name, int argsCount) {
        return reinterpret_cast<T>(get_method_pointer(assemblyName, namespaze, klassName, name,
                                                      argsCount));
    }

    const MethodInfo *get_method(const char *assemblyName, const char *namespaze,
                                 const char *klassName, const char *name, int argsCount);

    Il2CppClass *find_class(const char *assemblyName, const char *namespaze,
                            const std::function<bool(Il2CppClass *)> &predict);

    Il2CppMethodPointer find_method(const char *assemblyName, const char *namespaze,
                                    const char *klassName,
                                    const std::function<bool(const MethodInfo *)> &predict);
}

#endif //UMAMUSUMELOCALIFYANDROID_IL2CPP_SYMBOLS_H
