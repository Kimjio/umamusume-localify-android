#pragma once

#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <fstream>
#include <dobby.h>
#include <jni.h>
#include <pthread.h>
#include <unordered_map>

#include <rapidjson/document.h>
#include <rapidjson/encodings.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/stringbuffer.h>

#include "log.h"

#include "fnv1a_hash.hpp"

#include "game.hpp"

#include "il2cpp/il2cpp-class.h"

#if defined(__ARM_ARCH_7A__)
#define ABI "armeabi-v7a"
#elif defined(__i386__)
#define ABI "x86"
#elif defined(__x86_64__)
#define ABI "x86_64"
#elif defined(__aarch64__)
#define ABI "arm64-v8a"
#else
#define ABI "unknown"
#endif

struct ReplaceAsset {
    std::string path;
    Il2CppObject *asset;
};

using namespace std;

extern bool g_enable_logger;
extern int g_max_fps;
extern float g_ui_animation_scale;
extern bool g_ui_use_system_resolution;
extern float g_resolution_3d_scale;
extern bool g_replace_to_builtin_font;
extern bool g_replace_to_custom_font;
extern string g_font_assetbundle_path;
extern string g_font_asset_name;
extern string g_tmpro_font_asset_name;
extern bool g_dump_entries;
extern bool g_dump_db_entries;
extern bool g_static_entries_use_hash;
extern bool g_static_entries_use_text_id_name;
/**
 * -1 Auto (Default behavior)
 * 0 Toon1280, Anti: 0
 * 1 Toon1280x2, Anti: 2
 * 2 Toon1280x4, Anti: 4
 * 3 ToonFull, Anti: 8
 */
extern int g_graphics_quality;
/**
 * -1 Follow graphics quality
 * 0 MSAA OFF
 * 2 x2
 * 4 x4
 * 8 x8
 */
extern int g_anti_aliasing;
extern bool g_force_landscape;
extern float g_force_landscape_ui_scale;
extern bool g_ui_loading_show_orientation_guide;
extern bool g_restore_notification;
extern unordered_map<string, ReplaceAsset> g_replace_assets;
extern string g_replace_assetbundle_file_path;
extern string g_replace_text_db_path;
extern bool g_character_system_text_caption;
/*
 * ModeNormal 0
 * Mode60FPS 1
 * SkipFrame 2
 * SkipFramePostAlways 3
 */
extern int g_cyspring_update_mode;
extern bool g_hide_now_loading;
extern bool g_dump_msgpack;
extern bool g_dump_msgpack_request;
extern string g_packet_notifier;
extern bool g_restore_gallop_webview;
extern bool g_use_third_party_news;

namespace {
    // copy-pasted from https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
    void replaceAll(string &str, const string &from, const string &to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }

    int GetAndroidApiLevel() {
        return android_get_device_api_level();
    }

    string GetNativeBridgeLibrary() {
        auto nativeBridge = array<char, PROP_VALUE_MAX>();
        __system_property_get("ro.dalvik.vm.native.bridge", nativeBridge.data());
        return string(nativeBridge.data());
    }

    bool IsABIRequiredNativeBridge() {
        return ABI == "x86"s || ABI == "x86_64"s;
    }

    bool IsRunningOnNativeBridge() {
        auto systemAbi = array<char, PROP_VALUE_MAX>();
        __system_property_get("ro.product.cpu.abi", systemAbi.data());
        auto isaArm = array<char, PROP_VALUE_MAX>();
        __system_property_get("ro.dalvik.vm.isa.arm", isaArm.data());
        return ((systemAbi.data() == "x86"s || systemAbi.data() == "x86_64"s) || isaArm.data() == "x86"s) &&
               (ABI == "armeabi-v7a"s || ABI == "arm64-v8a"s);
    }
}
