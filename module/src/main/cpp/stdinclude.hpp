//
// Created by kimji on 2022-05-01.
//

#pragma once

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

extern bool g_enable_logger;
extern int g_max_fps;
extern float g_ui_animation_scale;
extern bool g_replace_to_builtin_font;
extern bool g_replace_to_custom_font;
extern std::string g_font_assetbundle_path;
extern std::string g_font_asset_name;
extern bool g_dump_entries;
extern bool g_dump_db_entries;

namespace {
    // copy-pasted from https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
    void replaceAll(std::string& str, const std::string& from, const std::string& to)
    {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }
}
