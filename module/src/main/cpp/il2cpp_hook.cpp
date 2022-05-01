//
// Created by kimji on 2022-05-01.
//
#include "stdinclude.hpp"
#include "il2cpp_hook.h"
#include "il2cpp/il2cpp_symbols.h"
#include "localify/localify.h"
#include "logger/logger.h"
#include <codecvt>

using namespace std;
using namespace il2cpp_symbols;
using namespace localify;
using namespace logger;

static void *il2cpp_handle = nullptr;
static uint64_t il2cpp_base = 0;


void* populate_with_errors_orig = nullptr;
bool populate_with_errors_hook(void* _this, Il2CppString* str, TextGenerationSettings_t* settings, void* context)
{
    return reinterpret_cast<decltype(populate_with_errors_hook)*>(populate_with_errors_orig) (
            _this, localify::get_localized_string(str), settings, context
    );
}

void* localize_get_orig = nullptr;
Il2CppString* localize_get_hook(int id)
{
    auto orig_result = reinterpret_cast<decltype(localize_get_hook)*>(localize_get_orig)(id);
    auto result = localify::get_localized_string(id);

    return result ? result : orig_result;
}

std::unordered_map<void*, bool> text_queries;

void* query_ctor_orig = nullptr;
void* query_ctor_hook(void* _this, void* conn, Il2CppString* sql)
{
    auto sqlQuery = localify::u16_wide(sql->start_char);

    if (sqlQuery.find(L"text_data") != std::string::npos ||
        sqlQuery.find(L"character_system_text") != std::string::npos ||
        sqlQuery.find(L"race_jikkyo_comment") != std::string::npos ||
        sqlQuery.find(L"race_jikkyo_message") != std::string::npos )
    {
        text_queries.emplace(_this, true);
    }

    return reinterpret_cast<decltype(query_ctor_hook)*>(query_ctor_orig)(_this, conn, sql);
}

void* query_dispose_orig = nullptr;
void query_dispose_hook(void* _this)
{
    if (text_queries.contains(_this))
        text_queries.erase(_this);

    return reinterpret_cast<decltype(query_dispose_hook)*>(query_dispose_orig)(_this);
}

void* query_getstr_orig = nullptr;
Il2CppString*  query_getstr_hook(void* _this, int32_t idx)
{
    auto result = reinterpret_cast<decltype(query_getstr_hook)*>(query_getstr_orig)(_this, idx);

    if (text_queries.contains(_this))
        return localify::get_localized_string(result);

    return result;
}

void (*text_assign_font)(void*);
int (*text_get_size)(void*);
void (*text_set_size)(void*, int);
float (*text_get_linespacing)(void*);
void (*text_set_style)(void*, int);
void (*text_set_linespacing)(void*, float);

void* on_populate_orig = nullptr;
void on_populate_hook(void* _this, void* toFill)
{
    if (text_get_linespacing(_this) != 1.05f)
    {
        text_assign_font(_this);
        text_set_style(_this, 1);
        text_set_size(_this, text_get_size(_this) - 4);
        text_set_linespacing(_this, 1.05f);
    }

    return reinterpret_cast<decltype(on_populate_hook)*>(on_populate_orig)(_this, toFill);
}

void* set_fps_orig = nullptr;
void set_fps_hook(int value)
{
    return reinterpret_cast<decltype(set_fps_hook)*>(set_fps_orig)(g_max_fps);
}

void dump_all_entries()
{
    // 0 is None
    for(size_t i = 1;;i++)
    {
        auto* str = reinterpret_cast<decltype(localize_get_hook)*>(localize_get_orig)(i);

        if (str && *str->start_char)
        {
            logger::write_entry(i, str->start_char);
        }
        else
        {
            // check next string, if it's still empty, then we are done!
            auto* nextStr = reinterpret_cast<decltype(localize_get_hook)*>(localize_get_orig)(i + 1);
            if (!(nextStr && *nextStr->start_char))
                break;
        }
    }
}

void init_il2cpp_api() {
#define DO_API(r, n, p) n = (r (*) p)dlsym(il2cpp_handle, #n)

#include "il2cpp/il2cpp-api-functions.h"

#undef DO_API
}

uint64_t get_module_base(const char *module_name) {
    uint64_t addr = 0;
    char line[1024];
    uint64_t start = 0;
    uint64_t end = 0;
    char flags[5];
    char path[PATH_MAX];

    FILE *fp = fopen("/proc/self/maps", "r");
    if (fp != nullptr) {
        while (fgets(line, sizeof(line), fp)) {
            strcpy(path, "");
            sscanf(line, "%" PRIx64"-%" PRIx64" %s %*" PRIx64" %*x:%*x %*u %s\n", &start, &end,
                   flags, path);
#if defined(__aarch64__)
            if (strstr(flags, "x") == 0)
                continue;
#endif
            if (strstr(path, module_name)) {
                addr = start;
                break;
            }
        }
        fclose(fp);
    }
    return addr;
}

void hookMethods() {
    auto populate_with_errors_addr = il2cpp_symbols::get_method_pointer(
            "UnityEngine.TextRenderingModule.dll",
            "UnityEngine", "TextGenerator",
            "PopulateWithErrors", 3
    );

    // have to do this way because there's Get(TextId id) and Get(string id)
    // the string one looks like will not be called by elsewhere
    auto localize_get_addr = il2cpp_symbols::find_method("umamusume.dll", "Gallop", "Localize", [](const MethodInfo* method) {
        return method->name == "Get"s &&
               method->parameters->parameter_type->type == IL2CPP_TYPE_VALUETYPE;
    });

    auto query_ctor_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Query", ".ctor", 2
    );

    auto query_getstr_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Query", "GetText", 1
    );

    auto query_dispose_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Query", "Dispose", 0
    );

    auto on_populate_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "OnPopulateMesh", 1
    );

    text_assign_font = reinterpret_cast<void(*)(void*)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "AssignDefaultFont", 0
            )
    );

    text_get_size = reinterpret_cast<int(*)(void*)>(
            il2cpp_symbols::get_method_pointer(
                    "umamusume.dll", "Gallop",
                    "TextCommon", "get_FontSize", 0
            )
    );

    text_set_size = reinterpret_cast<void(*)(void*, int)>(
            il2cpp_symbols::get_method_pointer(
                    "umamusume.dll", "Gallop",
                    "TextCommon", "set_FontSize", 1
            )
    );

    text_get_linespacing = reinterpret_cast<float(*)(void*)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "get_lineSpacing", 0
            )
    );

    text_set_style = reinterpret_cast<void(*)(void*, int)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_fontStyle", 1
            )
    );

    text_set_linespacing = reinterpret_cast<void(*)(void*, float)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_lineSpacing", 1
            )
    );

    auto set_fps_addr =
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.CoreModule.dll", "UnityEngine",
                    "Application", "set_targetFrameRate", 1);


#define ADD_HOOK(_name_) \
	DobbyHook((void *)_name_##_addr, (void *) _name_##_hook, (void **) &_name_##_orig);
#pragma endregion

    // hook UnityEngine.TextGenerator::PopulateWithErrors to modify text
    ADD_HOOK(populate_with_errors)

    // Looks like they store all localized texts that used by code in a dict
    ADD_HOOK(localize_get)

    ADD_HOOK(query_ctor)
    ADD_HOOK(query_getstr)
    ADD_HOOK(query_dispose)

    if (g_replace_font) {
        ADD_HOOK(on_populate)
    }

    if (g_max_fps > -1) {
        ADD_HOOK(set_fps)
    }

    if (g_dump_entries) {
        dump_all_entries();
    }

    if (g_dump_db_entries) {
        logger::dump_db_texts();
    }
}

void il2cpp_hook(void *handle) {
    //initialize
    LOGI("il2cpp_handle: %p", handle);
    il2cpp_handle = handle;
    init_il2cpp_api();
    if (il2cpp_domain_get_assemblies) {
        Dl_info dlInfo;
        if (dladdr((void *) il2cpp_domain_get_assemblies, &dlInfo)) {
            il2cpp_base = reinterpret_cast<uint64_t>(dlInfo.dli_fbase);
        } else {
            LOGW("dladdr error, using get_module_base.");
            il2cpp_base = get_module_base("libil2cpp.so");
        }
        LOGI("il2cpp_base: %" PRIx64"", il2cpp_base);
    } else {
        LOGE("Failed to initialize il2cpp api.");
        return;
    }
    auto domain = il2cpp_domain_get();
    il2cpp_thread_attach(domain);

    il2cpp_symbols::init(domain);
    hookMethods();
}
