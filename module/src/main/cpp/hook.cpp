//
// Created by Perfare on 2020/7/4.
//

#include <sstream>
#include <thread>
#include "stdinclude.hpp"
#include "hook.h"
#include "il2cpp_hook.h"
#include "game.hpp"
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

bool isGame(const char *pkgNm) {
    if (!pkgNm)
        return false;
    if (strcmp(pkgNm, GamePackageName) == 0) {
        gameRegion = GameRegion::JAP;
    }
    if (strcmp(pkgNm, GamePackageNameKor) == 0) {
        gameRegion = GameRegion::KOR;
    }
    if (gameRegion != GameRegion::UNKNOWN) {
        LOGI("detect game: %s", pkgNm);
        return true;
    }
    return false;
}

void dlopen_process(const char *name, void *handle) {
    if (!il2cpp_handle) {
        if (name != nullptr && strstr(name, "libil2cpp.so")) {
            il2cpp_handle = handle;
            LOGI("Got il2cpp handle!");
        }
    }
}

HOOK_DEF(void*, do_dlopen, const char *name, int flags) {
    void *handle = orig_do_dlopen(name, flags);
    dlopen_process(name, handle);
    return handle;
}

HOOK_DEF(void*, __loader_dlopen, const char *filename, int flags, const void *caller_addr) {
    void *handle = orig___loader_dlopen(filename, flags, caller_addr);
    dlopen_process(filename, handle);
    return handle;
}

HOOK_DEF(void*, do_dlopen_V24, const char *name, int flags, const void *extinfo [[maybe_unused]],
         void *caller_addr) {
    void *handle = orig_do_dlopen_V24(name, flags, extinfo, caller_addr);
    dlopen_process(name, handle);
    return handle;
}

HOOK_DEF(void*, do_dlopen_V19, const char *name, int flags, const void *extinfo [[maybe_unused]]) {
    void *handle = orig_do_dlopen_V19(name, flags, extinfo);
    dlopen_process(name, handle);
    return handle;
}

HOOK_DEF(void*, NativeBridgeLoadLibrary_V21, const char *filename, int flag) {
    if (string(filename).find(string("libmain.so")) != string::npos) {
        auto *NativeBridgeError = reinterpret_cast<bool (*)()>(DobbySymbolResolver(nullptr,
                                                                                   "_ZN7android17NativeBridgeErrorEv"));

        stringstream path_armV8;
        path_armV8 << "/data/data/" << GetCurrentPackageName().data() << "/arm64-v8a.so";
        stringstream path_armV7;
        path_armV7 << "/data/data/" << GetCurrentPackageName().data() << "/armeabi-v7a.so";

        char *path = nullptr;

        if (access(path_armV8.str().data(), F_OK) != -1) {
            path = path_armV8.str().data();
        } else if (access(path_armV7.str().data(), F_OK) != -1) {
            path = path_armV7.str().data();
        }
        if (path != nullptr) {
            std::thread load_thread([path, NativeBridgeError]() {
                void *lib = orig_NativeBridgeLoadLibrary_V21(path, RTLD_NOW);
                LOGI("%s: %p", path, lib);
                if (NativeBridgeError()) {
                    LOGW("LoadLibrary failed");
                }
                DobbyDestroy((void *) new_do_dlopen_V24);
            });
            load_thread.detach();
        }
    }

    return orig_NativeBridgeLoadLibrary_V21(filename, flag);
}

HOOK_DEF(void*, NativeBridgeLoadLibraryExt_V26, const char *filename, int flag,
         struct native_bridge_namespace_t *ns) {
    if (string(filename).find(string("libmain.so")) != string::npos) {
        auto *NativeBridgeError = reinterpret_cast<bool (*)()>(DobbySymbolResolver(nullptr,
                                                                                   "_ZN7android17NativeBridgeErrorEv"));
        auto *NativeBridgeGetError = reinterpret_cast<char *(*)()>(DobbySymbolResolver(nullptr,
                                                                                       "_ZN7android20NativeBridgeGetErrorEv"));

        stringstream path_armV8;
        path_armV8 << "/data/data/" << GetCurrentPackageName().data() << "/arm64-v8a.so";
        stringstream path_armV7;
        path_armV7 << "/data/data/" << GetCurrentPackageName().data() << "/armeabi-v7a.so";

        char *path = nullptr;

        if (access(path_armV8.str().data(), F_OK) != -1) {
            path = path_armV8.str().data();
        } else if (access(path_armV7.str().data(), F_OK) != -1) {
            path = path_armV7.str().data();
        }
        if (path != nullptr) {
            std::thread load_thread([path, ns, NativeBridgeError, NativeBridgeGetError]() {
                void *lib = orig_NativeBridgeLoadLibraryExt_V26(path, RTLD_NOW, ns);
                LOGI("%s: %p", path, lib);
                if (NativeBridgeError()) {
                    char *error_bridge;
                    if ((error_bridge = NativeBridgeGetError()) !=
                        nullptr) {
                        LOGW("error_bridge: %s", error_bridge);
                    }
                }
                DobbyDestroy((void *) new_do_dlopen_V24);
            });
            load_thread.detach();
        }
    }

    return orig_NativeBridgeLoadLibraryExt_V26(filename, flag, ns);
}

HOOK_DEF(void*, NativeBridgeLoadLibraryExt_V30, const char *filename, int flag,
         struct native_bridge_namespace_t *ns) {
    if (string(filename).find(string("libmain.so")) != string::npos) {
        auto *NativeBridgeError = reinterpret_cast<bool (*)()>(DobbySymbolResolver(nullptr,
                                                                                   "NativeBridgeError"));
        auto *NativeBridgeGetError = reinterpret_cast<char *(*)()>(DobbySymbolResolver(nullptr,
                                                                                       "NativeBridgeGetError"));

        stringstream path_armV8;
        path_armV8 << "/data/data/" << GetCurrentPackageName().data() << "/arm64-v8a.so";
        stringstream path_armV7;
        path_armV7 << "/data/data/" << GetCurrentPackageName().data() << "/armeabi-v7a.so";

        string path;

        if (access(path_armV8.str().data(), F_OK) != -1) {
            path = path_armV8.str();
        } else if (access(path_armV7.str().data(), F_OK) != -1) {
            path = path_armV7.str();
        }

        if (!path.empty()) {
            std::thread load_thread([path, ns, NativeBridgeError, NativeBridgeGetError]() {
                void *lib = orig_NativeBridgeLoadLibraryExt_V30(path.data(), RTLD_NOW, ns);
                LOGI("%s: %p", path.data(), lib);
                if (NativeBridgeError()) {
                    char *error_bridge;
                    if ((error_bridge = NativeBridgeGetError()) !=
                        nullptr) {
                        LOGW("error_bridge: %s", error_bridge);
                    }
                }
                DobbyDestroy((void *) new_do_dlopen_V24);
            });
            load_thread.detach();
        }
    }

    return orig_NativeBridgeLoadLibraryExt_V30(filename, flag, ns);
}

std::optional<std::vector<std::string>> read_config() {
    std::ifstream config_stream{
            string("/sdcard/Android/data/").append(GamePackageName).append("/config.json")};
    std::vector<std::string> dicts{};

    if (!config_stream.is_open()) {
        LOGW("config.json not loaded.");
        return nullopt;
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
        if (!document.HasMember("replaceFont") && document.HasMember("replaceToBuiltinFont")) {
            g_replace_to_builtin_font = document["replaceToBuiltinFont"].GetBool();
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

void hack_thread(void *arg [[maybe_unused]]) {
    LOGI("%s hack thread: %d", ABI, gettid());
    int api_level = GetAndroidApiLevel();
    LOGI("%s api level: %d", ABI, api_level);
    if (api_level >= 30) {
        if (!IsABIRequiredNativeBridge()) {
            void *addr;
            if (IsRunningOnNativeBridge()) {
                addr = (void *) dlopen;
            } else {
                addr = DobbySymbolResolver(nullptr,
                                           "__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv");
            }
            if (addr) {
                LOGI("%s do_dlopen at: %p", ABI, addr);
                if (IsRunningOnNativeBridge()) {
                    DobbyHook(addr, (void *) new_do_dlopen,
                              (void **) &orig_do_dlopen);
                } else {
                    DobbyHook(addr, (void *) new_do_dlopen_V24,
                              (void **) &orig_do_dlopen_V24);
                }
            }
        } else {
            void *addr = DobbySymbolResolver(nullptr,
                                             "NativeBridgeLoadLibraryExt");
            if (addr) {
                LOGI("NativeBridgeLoadLibraryExt at: %p", addr);
                DobbyHook(addr, (void *) new_NativeBridgeLoadLibraryExt_V30,
                          (void **) &orig_NativeBridgeLoadLibraryExt_V30);
            }
        }
    } else if (api_level >= 26) {
        if (!IsABIRequiredNativeBridge()) {
            void *addr;
            if (IsRunningOnNativeBridge()) {
                addr = (void *) dlopen;
            } else {
                void *libdl_handle = dlopen("libdl.so", RTLD_LAZY);
                addr = dlsym(libdl_handle, "__loader_dlopen");
            }
            if (addr) {
                if (IsRunningOnNativeBridge()) {
                    LOGI("do_dlopen at: %p", addr);
                    DobbyHook(addr, (void *) new_do_dlopen,
                              (void **) &orig_do_dlopen);
                } else {
                    LOGI("__loader_dlopen at: %p", addr);
                    DobbyHook(addr, (void *) new___loader_dlopen,
                              (void **) &orig___loader_dlopen);
                }
            }
        } else {
            void *addr = DobbySymbolResolver(nullptr,
                                             "_ZN7android26NativeBridgeLoadLibraryExtEPKciPNS_25native_bridge_namespace_tE");
            if (addr) {
                LOGI("NativeBridgeLoadLibraryExt at: %p", addr);
                DobbyHook(addr, (void *) new_NativeBridgeLoadLibraryExt_V26,
                          (void **) &orig_NativeBridgeLoadLibraryExt_V26);
            }
        }
    } else if (api_level >= 24) {
        if (!IsABIRequiredNativeBridge()) {
            void *addr;
            if (IsRunningOnNativeBridge()) {
                addr = (void *) dlopen;
            } else {
                addr = DobbySymbolResolver(nullptr,
                                           "__dl__Z9do_dlopenPKciPK17android_dlextinfoPv");
            }
            if (addr) {
                LOGI("do_dlopen at: %p", addr);
                DobbyHook(addr, (void *) new_do_dlopen_V24,
                          (void **) &orig_do_dlopen_V24);
            }
        } else {
            void *addr = DobbySymbolResolver(nullptr,
                                             "_ZN7android23NativeBridgeLoadLibraryEPKci");
            if (addr) {
                LOGI("NativeBridgeLoadLibrary at: %p", addr);
                DobbyHook(addr, (void *) new_NativeBridgeLoadLibrary_V21,
                          (void **) &orig_NativeBridgeLoadLibrary_V21);
            }
        }
    } else {
        if (!IsABIRequiredNativeBridge()) {
            void *addr = DobbySymbolResolver(nullptr,
                                             "__dl__Z9do_dlopenPKciPK17android_dlextinfo");
            if (addr) {
                LOGI("do_dlopen at: %p", addr);
                DobbyHook(addr, (void *) new_do_dlopen_V19,
                          (void **) &orig_do_dlopen_V19);
            }
        } else {
            void *addr = DobbySymbolResolver(nullptr,
                                             "_ZN7android23NativeBridgeLoadLibraryEPKci");
            if (addr) {
                LOGI("NativeBridgeLoadLibrary at: %p", addr);
                DobbyHook(addr, (void *) new_NativeBridgeLoadLibrary_V21,
                          (void **) &orig_NativeBridgeLoadLibrary_V21);
            }
        }
    }
    if (IsABIRequiredNativeBridge()) {
        return;
    }
    while (!il2cpp_handle) {
        sleep(1);
    }

    auto dict = read_config();

    if (!dict.has_value()) {
        return;
    }

    std::thread init_thread([dict]() {
        logger::init_logger();
        localify::load_textdb(&dict.value());
        il2cpp_hook(il2cpp_handle);
    });
    init_thread.detach();
}