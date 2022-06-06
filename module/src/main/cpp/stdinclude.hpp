//
// Created by kimji on 2022-05-01.
//

#pragma once

#define LOGGING_DEBUG true

#include <cstdio>
#include <unistd.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>
#include <cinttypes>
#include <string>
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

using namespace std;

extern bool g_enable_logger;
extern int g_max_fps;
extern float g_ui_animation_scale;
extern bool g_replace_to_builtin_font;
extern bool g_replace_to_custom_font;
extern string g_font_assetbundle_path;
extern string g_font_asset_name;
extern bool g_dump_entries;
extern bool g_dump_db_entries;

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

    bool IsABIRequiredNativeBridge() {
        return ABI == "x86"s || ABI == "x86_64"s;
    }

    bool IsRunningOnNativeBridge() {
        char systemAbi[PROP_VALUE_MAX];
        __system_property_get("ro.product.cpu.abi", systemAbi);
        return (systemAbi == "x86"s || systemAbi == "x86_64"s) &&
               (ABI == "armeabi-v7a"s || ABI == "arm64-v8a"s);
    }
}
