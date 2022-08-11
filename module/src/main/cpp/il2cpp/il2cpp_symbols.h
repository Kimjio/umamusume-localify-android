#ifndef UMAMUSUMELOCALIFYANDROID_IL2CPP_SYMBOLS_H
#define UMAMUSUMELOCALIFYANDROID_IL2CPP_SYMBOLS_H

#include "../stdinclude.hpp"
#include "il2cpp-class.h"

#define DO_API(r, n, p) inline r (*n) p

#include "il2cpp-api-functions.h"

#undef DO_API

namespace il2cpp_symbols
{
    void init(Il2CppDomain* domain);

    Il2CppClass* get_class(const char* assemblyName, const char* namespaze, const char* klassName);

    Il2CppMethodPointer get_method_pointer(const char* assemblyName, const char* namespaze,
                                 const char* klassName, const char* name, int argsCount);

    const MethodInfo* get_method(const char* assemblyName, const char* namespaze,
                           const char* klassName, const char* name, int argsCount);

    Il2CppMethodPointer find_method(const char* assemblyName, const char* namespaze,
                          const char* klassName, std::function<bool(const MethodInfo*)> predict);
}

#endif //UMAMUSUMELOCALIFYANDROID_IL2CPP_SYMBOLS_H
