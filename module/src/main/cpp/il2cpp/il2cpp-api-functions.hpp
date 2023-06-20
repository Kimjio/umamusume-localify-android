template<typename T>
T il2cpp_resolve_icall_t(const char *name) {
    return reinterpret_cast<T>(il2cpp_resolve_icall(name));
}

template<typename T>
T il2cpp_object_new_t(const Il2CppClass *klass) {
    return reinterpret_cast<T>(il2cpp_object_new(klass));
}

template<typename T>
T il2cpp_object_unbox_t(Il2CppObject *obj) {
    return reinterpret_cast<T>(il2cpp_object_unbox(obj));
}

template<typename T>
T il2cpp_value_box_t(Il2CppClass *klass, void *data) {
    return reinterpret_cast<T>(il2cpp_value_box(klass, data));
}
