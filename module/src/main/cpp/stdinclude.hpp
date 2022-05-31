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
