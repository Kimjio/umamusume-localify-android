//
// Created by Perfare on 2020/7/4.
//

#include <iostream>
#include <thread>
#include "stdinclude.hpp"
#include "hook.h"
#include "il2cpp_hook.h"
#include "game.h"
#include "localify/localify.h"
#include "logger/logger.h"

using namespace std;
using namespace localify;
using namespace logger;

bool g_enable_logger = false;
int g_max_fps = -1;
float g_ui_animation_scale = 1.0f;
bool g_replace_to_builtin_font = true;
bool g_replace_to_custom_font = false;
std::string g_font_assetbundle_path;
std::string g_font_asset_name;
bool g_dump_entries = false;
bool g_dump_db_entries = false;

int isGame(JNIEnv *env, jstring appDataDir) {
    if (!appDataDir)
        return 0;
    const char *app_data_dir = env->GetStringUTFChars(appDataDir, nullptr);
    int user = 0;
    static char package_name[256];
    if (sscanf(app_data_dir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
        if (sscanf(app_data_dir, "/data/%*[^/]/%s", package_name) != 1) {
            package_name[0] = '\0';
            LOGW("can't parse %s", app_data_dir);
            return 0;
        }
    }
    if (strcmp(package_name, GamePackageName) == 0) {
        LOGI("detect game: %s", package_name);
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 1;
    } else {
        env->ReleaseStringUTFChars(appDataDir, app_data_dir);
        return 0;
    }
}

static int GetAndroidApiLevel() {
    char prop_value[PROP_VALUE_MAX];
    __system_property_get("ro.build.version.sdk", prop_value);
    return atoi(prop_value);
}

void dlopen_process(const char *name, void *handle) {
    if (!il2cpp_handle) {
        if (name != nullptr && strstr(name, "libil2cpp.so")) {
            il2cpp_handle = handle;
            LOGI("Got il2cpp handle!");
        }
    }
}

HOOK_DEF(void*, __loader_dlopen, const char *filename, int flags, const void *caller_addr) {
    void *handle = orig___loader_dlopen(filename, flags, caller_addr);
    dlopen_process(filename, handle);
    return handle;
}

HOOK_DEF(void*, do_dlopen_V24, const char *name, int flags, const void *extinfo,
         void *caller_addr) {
    void *handle = orig_do_dlopen_V24(name, flags, extinfo, caller_addr);
    dlopen_process(name, handle);
    return handle;
}

HOOK_DEF(void*, do_dlopen_V19, const char *name, int flags, const void *extinfo) {
    void *handle = orig_do_dlopen_V19(name, flags, extinfo);
    dlopen_process(name, handle);
    return handle;
}

std::vector<std::string> read_config() {
    std::ifstream config_stream{string("/sdcard/Android/data/").append(GamePackageName).append("/config.json")};
    std::vector<std::string> dicts{};

    if (!config_stream.is_open()) {
        LOGW("config.json not loaded.");
        return dicts;
    }

    LOGI("config.json loaded.");

    rapidjson::IStreamWrapper wrapper{config_stream};
    rapidjson::Document document;

    document.ParseStream(wrapper);

    if (!document.HasParseError()) {
        if (document.HasMember("enableLogger")) {
            g_enable_logger = document["enableLogger"].GetBool();
        }
        if (document.HasMember("dumpStaticEntries")) {
            g_dump_entries = document["dumpStaticEntries"].GetBool();
        }
        if (document.HasMember("maxFps")) {
            g_max_fps = document["maxFps"].GetInt();
        }
        if (document.HasMember("uiAnimationScale")) {
            g_ui_animation_scale = document["uiAnimationScale"].GetFloat();
        }
        if (document.HasMember("replaceFont")) {
            g_replace_to_builtin_font = document["replaceFont"].GetBool();
        }
        if (document.HasMember("replaceToCustomFont")) {
            g_replace_to_custom_font = document["replaceToCustomFont"].GetBool();
        }
        if (document.HasMember("fontAssetBundlePath")) {
            g_font_assetbundle_path = std::string(document["fontAssetBundlePath"].GetString());
        }
        if (document.HasMember("fontAssetName")) {
            g_font_asset_name = std::string(document["fontAssetName"].GetString());
        }

        auto &dicts_arr = document["dicts"];
        auto len = dicts_arr.Size();

        for (size_t i = 0; i < len; ++i) {
            auto dict = dicts_arr[i].GetString();

            dicts.emplace_back(dict);
        }
    }

    config_stream.close();
    return dicts;
}

void *hack_thread(void *arg) {
    LOGI("hack thread: %d", gettid());
    int api_level = GetAndroidApiLevel();
    LOGI("api level: %d", api_level);
    if (api_level >= 30) {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");
        if (addr) {
            LOGI("do_dlopen at: %p", addr);
            DobbyHook(addr, (void *) new_do_dlopen_V24,
                      (void **) &orig_do_dlopen_V24);
        }
    } else if (api_level >= 26) {
        void *libdl_handle = dlopen("libdl.so", RTLD_LAZY);
        void *addr = dlsym(libdl_handle, "__loader_dlopen");
        LOGI("__loader_dlopen at: %p", addr);
        DobbyHook(addr, (void *) new___loader_dlopen,
                  (void **) &orig___loader_dlopen);
    } else if (api_level >= 24) {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfoPv");
        if (addr) {
            LOGI("do_dlopen at: %p", addr);
            DobbyHook(addr, (void *) new_do_dlopen_V24,
                      (void **) &orig_do_dlopen_V24);
        }
    } else {
        void *addr = DobbySymbolResolver(nullptr,
                                         "__dl__Z9do_dlopenPKciPK17android_dlextinfo");
        if (addr) {
            LOGI("do_dlopen at: %p", addr);
            DobbyHook(addr, (void *) new_do_dlopen_V19,
                      (void **) &orig_do_dlopen_V19);
        }
    }
    while (!il2cpp_handle) {
        sleep(1);
    }

    auto dict = read_config();

    std::thread init_thread([dict]() {
        logger::init_logger();
        localify::load_textdb(&dict);
        il2cpp_hook(il2cpp_handle);
    });
    init_thread.detach();

    return nullptr;
}