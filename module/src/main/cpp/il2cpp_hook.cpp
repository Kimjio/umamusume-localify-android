//
// Created by kimji on 2022-05-01.
//
#include "stdinclude.hpp"
#include "il2cpp_hook.h"
#include "il2cpp/il2cpp_symbols.h"
#include "localify/localify.h"
#include "logger/logger.h"
#include <codecvt>
#include <thread>

using namespace std;
using namespace il2cpp_symbols;
using namespace localify;
using namespace logger;

static void *il2cpp_handle = nullptr;
static uint64_t il2cpp_base = 0;

Il2CppObject *assets = nullptr;

Il2CppObject *(*load_assets)(Il2CppObject *thisObj, Il2CppString *name, const Il2CppType *type);

Il2CppString *(*uobject_get_name)(Il2CppObject *uObject);

const Il2CppType *
GetRuntimeType(const char *assemblyName, const char *namespaze, const char *klassName) {
    auto dummyObj = (Il2CppObject *) il2cpp_object_new(
            il2cpp_symbols::get_class(assemblyName, namespaze, klassName));
    auto (*get_type)(Il2CppObject *thisObj) = reinterpret_cast<const Il2CppType *(*)(
            Il2CppObject *thisObj)>(il2cpp_symbols::get_method_pointer("mscorlib.dll", "System",
                                                                       "Object", "GetType", 0));
    return get_type(dummyObj);
}

Il2CppObject *GetCustomFont() {
    if (!assets) return nullptr;
    return load_assets(assets, il2cpp_string_new(g_font_asset_name.data()),
                       GetRuntimeType("UnityEngine.TextRenderingModule.dll", "UnityEngine",
                                      "Font"));
}

void *populate_with_errors_orig = nullptr;

bool populate_with_errors_hook(void *thisObj, Il2CppString *str, TextGenerationSettings_t *settings,
                               void *context) {
    return reinterpret_cast<decltype(populate_with_errors_hook) *>(populate_with_errors_orig)(
            thisObj, localify::get_localized_string(str), settings, context
    );
}


void *localizeextension_text_orig = nullptr;

Il2CppString *localizeextension_text_hook(int id) {
    return localify::get_localized_string(id);
}

void *get_preferred_width_orig = nullptr;

float
get_preferred_width_hook(void *thisObj, Il2CppString *str, TextGenerationSettings_t *settings) {
    return reinterpret_cast<decltype(get_preferred_width_hook) *>(get_preferred_width_orig)(
            thisObj, localify::get_localized_string(str), settings
    );
}

void *localize_get_orig = nullptr;

Il2CppString *localize_get_hook(int id) {
    auto orig_result = reinterpret_cast<decltype(localize_get_hook) *>(localize_get_orig)(id);
    auto result = localify::get_localized_string(id);

    return result ? result : orig_result;
}

void ReplaceTextMeshFont(Il2CppObject *textMesh, Il2CppObject *meshRenderer) {
    Il2CppObject *font = GetCustomFont();
    Il2CppObject *fontMaterial = reinterpret_cast<Il2CppObject *(*)(
            Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(font->klass, "get_material",
                                                                      0)->methodPointer)(font);
    Il2CppObject *fontTexture = reinterpret_cast<Il2CppObject *(*)(
            Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(fontMaterial->klass,
                                                                      "get_mainTexture",
                                                                      0)->methodPointer)(
            fontMaterial);

    reinterpret_cast<void (*)(Il2CppObject *thisObj,
                              Il2CppObject *font)>(il2cpp_class_get_method_from_name(
            textMesh->klass, "set_font", 1)->methodPointer)(textMesh, font);
    if (meshRenderer) {
        auto get_sharedMaterial = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(meshRenderer->klass,
                                                                          "GetSharedMaterial",
                                                                          0)->methodPointer);

        Il2CppObject *sharedMaterial = get_sharedMaterial(meshRenderer);
        reinterpret_cast<void (*)(Il2CppObject *thisObj,
                                  Il2CppObject *texture)>(il2cpp_class_get_method_from_name(
                sharedMaterial->klass, "set_mainTexture", 1)->methodPointer)(sharedMaterial,
                                                                             fontTexture);
    }
}

void *an_text_set_material_to_textmesh_orig = nullptr;

void an_text_set_material_to_textmesh_hook(Il2CppObject *thisObj) {
    reinterpret_cast<decltype(an_text_set_material_to_textmesh_hook) *>(an_text_set_material_to_textmesh_orig)(
            thisObj);
    if (!assets) return;

    FieldInfo *mainField = il2cpp_class_get_field_from_name(thisObj->klass, "_mainTextMesh");
    FieldInfo *mainRenderer = il2cpp_class_get_field_from_name(thisObj->klass,
                                                               "_mainTextMeshRenderer");

    FieldInfo *outlineField = il2cpp_class_get_field_from_name(thisObj->klass,
                                                               "_outlineTextMeshList"); //List<TextMesh>
    FieldInfo *outlineFieldRenderers = il2cpp_class_get_field_from_name(thisObj->klass,
                                                                        "_outlineTextMeshRendererList"); //List<MeshRenderer>

    FieldInfo *shadowField = il2cpp_class_get_field_from_name(thisObj->klass, "_shadowTextMesh");
    FieldInfo *shadowFieldRenderer = il2cpp_class_get_field_from_name(thisObj->klass,
                                                                      "_shadowTextMeshRenderer");

    Il2CppObject *mainMesh;
    Il2CppObject *mainMeshRenderer;

    Il2CppObject *outlineMeshes;
    Il2CppObject *outlineMeshRenderers;

    Il2CppObject *shadowMesh;
    Il2CppObject *shadowMeshRenderer;

    il2cpp_field_get_value(thisObj, mainField, &mainMesh);
    il2cpp_field_get_value(thisObj, mainRenderer, &mainMeshRenderer);

    ReplaceTextMeshFont(mainMesh, mainMeshRenderer);

    vector<Il2CppObject *> textMeshies;
    il2cpp_field_get_value(thisObj, outlineField, &outlineMeshes);
    if (outlineMeshes) {
        FieldInfo *itemsField = il2cpp_class_get_field_from_name(outlineMeshes->klass, "_items");
        Il2CppArray *arr;
        il2cpp_field_get_value(outlineMeshes, itemsField, &arr);
        if (arr) {
            for (int i = 0; i < arr->max_length; i++) {
                auto *mesh = reinterpret_cast<Il2CppObject *>(arr->vector[i]);
                if (!mesh) {
                    break;
                }
                textMeshies.push_back(mesh);
            }
        }
    }

    il2cpp_field_get_value(thisObj, outlineFieldRenderers, &outlineMeshRenderers);
    if (outlineMeshRenderers) {
        FieldInfo *itemsField = il2cpp_class_get_field_from_name(outlineMeshRenderers->klass,
                                                                 "_items");
        Il2CppArray *arr;
        il2cpp_field_get_value(outlineMeshRenderers, itemsField, &arr);
        if (arr) {
            for (int i = 0; i < textMeshies.size(); i++) {
                auto *meshRenderer = reinterpret_cast<Il2CppObject *>(arr->vector[i]);
                ReplaceTextMeshFont(textMeshies[i], meshRenderer);
            }
        }
    }

    il2cpp_field_get_value(thisObj, shadowField, &shadowMesh);
    if (shadowMesh) {
        il2cpp_field_get_value(thisObj, shadowFieldRenderer, &shadowMeshRenderer);
        ReplaceTextMeshFont(shadowMesh, shadowMeshRenderer);
    }
}

void *an_text_fix_data_orig = nullptr;

void an_text_fix_data_hook(Il2CppObject *thisObj) {
    reinterpret_cast<decltype(an_text_fix_data_hook) *>(an_text_fix_data_orig)(thisObj);
    FieldInfo *field = il2cpp_class_get_field_from_name(thisObj->klass, "_text");
    Il2CppString *str;
    il2cpp_field_get_value(thisObj, field, &str);
    il2cpp_field_set_value(thisObj, field, localify::get_localized_string(str));
}

void *update_orig = nullptr;

void *update_hook(Il2CppObject *thisObj, void *updateType, float deltaTime, float independentTime) {
    return reinterpret_cast<decltype(update_hook) *>(update_orig)(thisObj, updateType,
                                                                  deltaTime * g_ui_animation_scale,
                                                                  independentTime *
                                                                  g_ui_animation_scale);
}

std::unordered_map<void *, bool> text_queries;

void *query_ctor_orig = nullptr;

void *query_ctor_hook(void *thisObj, void *conn, Il2CppString *sql) {
    auto sqlQuery = localify::u16_wide(sql->start_char);

    if (sqlQuery.find(L"text_data") != std::string::npos ||
        sqlQuery.find(L"character_system_text") != std::string::npos ||
        sqlQuery.find(L"race_jikkyo_comment") != std::string::npos ||
        sqlQuery.find(L"race_jikkyo_message") != std::string::npos) {
        text_queries.emplace(thisObj, true);
    }

    return reinterpret_cast<decltype(query_ctor_hook) *>(query_ctor_orig)(thisObj, conn, sql);
}

void *query_dispose_orig = nullptr;

void query_dispose_hook(void *thisObj) {
    if (text_queries.contains(thisObj))
        text_queries.erase(thisObj);

    return reinterpret_cast<decltype(query_dispose_hook) *>(query_dispose_orig)(thisObj);
}

void *query_getstr_orig = nullptr;

Il2CppString *query_getstr_hook(void *thisObj, int32_t idx) {
    auto result = reinterpret_cast<decltype(query_getstr_hook) *>(query_getstr_orig)(thisObj, idx);

    if (text_queries.contains(thisObj))
        return localify::get_localized_string(result);

    return result;
}

void *story_timeline_controller_play_orig;

void *story_timeline_controller_play_hook(Il2CppObject *thisObj) {
    FieldInfo *timelineDataField = il2cpp_class_get_field_from_name(thisObj->klass,
                                                                    "_timelineData");

    Il2CppObject *timelineData;
    il2cpp_field_get_value(thisObj, timelineDataField, &timelineData);
    FieldInfo *StoryTimelineDataClass_TitleField = il2cpp_class_get_field_from_name(
            timelineData->klass, "Title");
    FieldInfo *StoryTimelineDataClass_BlockListField = il2cpp_class_get_field_from_name(
            timelineData->klass, "BlockList");

    Il2CppClass *story_timeline_text_clip_data_class = il2cpp_symbols::get_class("umamusume.dll",
                                                                                 "Gallop",
                                                                                 "StoryTimelineTextClipData");
    FieldInfo *nameField = il2cpp_class_get_field_from_name(story_timeline_text_clip_data_class,
                                                            "Name");
    FieldInfo *textField = il2cpp_class_get_field_from_name(story_timeline_text_clip_data_class,
                                                            "Text");
    FieldInfo *choiceDataListField = il2cpp_class_get_field_from_name(
            story_timeline_text_clip_data_class, "ChoiceDataList");
    FieldInfo *colorTextInfoListField = il2cpp_class_get_field_from_name(
            story_timeline_text_clip_data_class, "ColorTextInfoList");

    Il2CppString *title;
    il2cpp_field_get_value(timelineData, StoryTimelineDataClass_TitleField, &title);
    il2cpp_field_set_value(timelineData, StoryTimelineDataClass_TitleField,
                           localify::get_localized_string(title));

    Il2CppObject *blockList;
    il2cpp_field_get_value(timelineData, StoryTimelineDataClass_BlockListField, &blockList);

    Il2CppArray *blockArray;
    il2cpp_field_get_value(blockList, il2cpp_class_get_field_from_name(blockList->klass, "_items"),
                           &blockArray);

    for (size_t blockArrayIndex = 0; blockArrayIndex < blockArray->max_length; blockArrayIndex++) {
        auto *blockData = reinterpret_cast<Il2CppObject *>(blockArray->vector[blockArrayIndex]);
        if (!blockData) break;
        FieldInfo *StoryTimelineBlockDataClass_trackListField = il2cpp_class_get_field_from_name(
                blockData->klass, "_trackList");
        Il2CppObject *trackList;
        il2cpp_field_get_value(blockData, StoryTimelineBlockDataClass_trackListField, &trackList);

        Il2CppArray *trackArray;
        il2cpp_field_get_value(trackList,
                               il2cpp_class_get_field_from_name(trackList->klass, "_items"),
                               &trackArray);

        for (size_t trackArrayIndex = 0;
             trackArrayIndex < trackArray->max_length; trackArrayIndex++) {
            auto *trackData = reinterpret_cast<Il2CppObject *>(trackArray->vector[trackArrayIndex]);
            if (!trackData) break;
            FieldInfo *StoryTimelineTrackDataClass_ClipListField = il2cpp_class_get_field_from_name(
                    trackData->klass, "ClipList");
            Il2CppObject *clipList;
            il2cpp_field_get_value(trackData, StoryTimelineTrackDataClass_ClipListField, &clipList);


            Il2CppArray *clipArray;
            il2cpp_field_get_value(clipList,
                                   il2cpp_class_get_field_from_name(clipList->klass, "_items"),
                                   &clipArray);

            for (size_t clipArrayIndex = 0;
                 clipArrayIndex < clipArray->max_length; clipArrayIndex++) {
                auto *clipData = reinterpret_cast<Il2CppObject *>(clipArray->vector[clipArrayIndex]);
                if (!clipData) break;
                if (story_timeline_text_clip_data_class == clipData->klass) {
                    Il2CppString *name;
                    il2cpp_field_get_value(clipData, nameField, &name);
                    il2cpp_field_set_value(clipData, nameField,
                                           localify::get_localized_string(name));
                    Il2CppString *text;
                    il2cpp_field_get_value(clipData, textField, &text);
                    il2cpp_field_set_value(clipData, textField,
                                           localify::get_localized_string(text));
                    Il2CppObject *choiceDataList;
                    il2cpp_field_get_value(clipData, choiceDataListField, &choiceDataList);
                    if (choiceDataList) {
                        Il2CppArray *choiceDataArray;
                        il2cpp_field_get_value(choiceDataList, il2cpp_class_get_field_from_name(
                                choiceDataList->klass, "_items"), &choiceDataArray);

                        for (size_t i = 0; i < choiceDataArray->max_length; i++) {
                            auto *choiceData = reinterpret_cast<Il2CppObject *>(choiceDataArray->vector[i]);
                            if (!choiceData) break;
                            FieldInfo *choiceDataTextField = il2cpp_class_get_field_from_name(
                                    choiceData->klass, "Text");

                            Il2CppString *choiceDataText;
                            il2cpp_field_get_value(choiceData, choiceDataTextField,
                                                   &choiceDataText);
                            il2cpp_field_set_value(choiceData, choiceDataTextField,
                                                   localify::get_localized_string(choiceDataText));
                        }
                    }
                    Il2CppObject *colorTextInfoList;
                    il2cpp_field_get_value(clipData, colorTextInfoListField, &colorTextInfoList);
                    if (colorTextInfoList) {
                        Il2CppArray *colorTextInfoArray;
                        il2cpp_field_get_value(colorTextInfoList, il2cpp_class_get_field_from_name(
                                colorTextInfoList->klass, "_items"), &colorTextInfoArray);

                        for (size_t i = 0; i < colorTextInfoArray->max_length; i++) {
                            auto *colorTextInfo = reinterpret_cast<Il2CppObject *>(colorTextInfoArray->vector[i]);
                            if (!colorTextInfo) break;
                            FieldInfo *colorTextInfoTextField = il2cpp_class_get_field_from_name(
                                    colorTextInfo->klass, "Text");

                            Il2CppString *colorTextInfoText;
                            il2cpp_field_get_value(colorTextInfo, colorTextInfoTextField,
                                                   &colorTextInfoText);
                            il2cpp_field_set_value(colorTextInfo, colorTextInfoTextField,
                                                   localify::get_localized_string(
                                                           colorTextInfoText));
                        }
                    }
                }

            }
        }
    }

    return reinterpret_cast<decltype(story_timeline_controller_play_hook) *>(story_timeline_controller_play_orig)(
            thisObj);
}

void *story_race_textasset_load_orig;

void story_race_textasset_load_hook(Il2CppObject *thisObj) {
    FieldInfo *textDataField = {il2cpp_class_get_field_from_name(thisObj->klass, "textData")};
    Il2CppObject *textData;
    il2cpp_field_get_value(thisObj, textDataField, &textData);

    auto enumerator = reinterpret_cast<Il2CppObject *(*)(
            Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(textData->klass,
                                                                      "GetEnumerator",
                                                                      0)->methodPointer)(textData);
    auto get_current = reinterpret_cast<Il2CppObject *(*)(
            Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(enumerator->klass,
                                                                      "get_Current",
                                                                      0)->methodPointer);
    auto move_next = reinterpret_cast<bool (*)(
            Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(enumerator->klass, "MoveNext",
                                                                      0)->methodPointer);

    while (move_next(enumerator)) {
        auto key = get_current(enumerator);
        FieldInfo *textField = {il2cpp_class_get_field_from_name(key->klass, "text")};
        Il2CppString *text;
        il2cpp_field_get_value(key, textField, &text);
        il2cpp_field_set_value(key, textField, localify::get_localized_string(text));
    }

    reinterpret_cast<decltype(story_race_textasset_load_hook) *>(story_race_textasset_load_orig)(
            thisObj);
}

void (*text_assign_font)(void *);

void (*text_set_font)(void *, Il2CppObject *);

Il2CppObject *(*text_get_font)(void *);

int (*text_get_size)(void *);

void (*text_set_size)(void *, int);

float (*text_get_linespacing)(void *);

void (*text_set_style)(void *, int);

void (*text_set_linespacing)(void *, float);

Il2CppString *(*text_get_text)(void *);

void (*text_set_text)(void *, Il2CppString *);

void *on_populate_orig = nullptr;

void on_populate_hook(Il2CppObject *thisObj, void *toFill) {
    if (g_replace_to_builtin_font && text_get_linespacing(thisObj) != 1.05f) {
        text_set_style(thisObj, 1);
        text_set_size(thisObj, text_get_size(thisObj) - 4);
        text_set_linespacing(thisObj, 1.05f);
    }
    if (g_replace_to_custom_font) {
        auto font = text_get_font(thisObj);
        Il2CppString *name = uobject_get_name(font);
        if (g_font_asset_name.find(localify::u16_u8(name->start_char)) == string::npos) {
            text_set_font(thisObj, GetCustomFont());
        }
    }

    return reinterpret_cast<decltype(on_populate_hook) *>(on_populate_orig)(thisObj, toFill);
}

void *textcommon_awake_orig = nullptr;

void textcommon_awake_hook(Il2CppObject *thisObj) {
    if (g_replace_to_builtin_font) {
        text_assign_font(thisObj);
    }
    if (g_replace_to_custom_font) {
        auto customFont = GetCustomFont();
        if (customFont) {
            text_set_font(thisObj, customFont);
        }
    }
    text_set_text(thisObj, localify::get_localized_string(text_get_text(thisObj)));
    reinterpret_cast<decltype(textcommon_awake_hook) *>(textcommon_awake_orig)(thisObj);
}

void *get_modified_string_orig = nullptr;

Il2CppString *get_modified_string_hook(Il2CppString *text, Il2CppObject *input, bool allowNewLine) {
    if (!allowNewLine) {
        auto u8str = localify::u16_u8(text->start_char);
        replaceAll(u8str, "\n", "");
        return il2cpp_string_new(u8str.data());
    }
    return text;
}

void *set_fps_orig = nullptr;

void set_fps_hook([[maybe_unused]] int value) {
    return reinterpret_cast<decltype(set_fps_hook) *>(set_fps_orig)(g_max_fps);
}

void *load_zekken_composite_resource_orig = nullptr;

void load_zekken_composite_resource_hook(Il2CppObject *thisObj) {
    if (assets && g_replace_to_custom_font) {
        auto font = GetCustomFont();
        if (font) {
            FieldInfo *zekkenFontField = il2cpp_class_get_field_from_name(thisObj->klass,
                                                                          "_fontZekken");
            il2cpp_field_set_value(thisObj, zekkenFontField, font);
        }
    }
    reinterpret_cast<decltype(load_zekken_composite_resource_hook) *>(load_zekken_composite_resource_orig)(
            thisObj);
}

void *wait_resize_ui_orig = nullptr;

Il2CppObject *
wait_resize_ui_hook(Il2CppObject *thisObj, bool isPortrait, bool isShowOrientationGuide) {
    Il2CppObject *enumerator = reinterpret_cast<decltype(wait_resize_ui_hook) *>(wait_resize_ui_orig)(
            thisObj, isPortrait, isShowOrientationGuide);
    auto move_next = reinterpret_cast<void *(*)(
            Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(enumerator->klass, "MoveNext",
                                                                      0)->methodPointer);
    /*if (thisObj != NULL)
    {
        *(int*)((uint64_t)thisObj + 0x4C) = 1;
    }*/
    move_next(thisObj);
    return enumerator;
}

void dump_all_entries() {
    // 0 is None
    for (int i = 1;; i++) {
        auto *str = reinterpret_cast<decltype(localize_get_hook) *>(localize_get_orig)(i);

        if (str && *str->start_char) {
            logger::write_entry(i, str->start_char);
        } else {
            // check next string, if it's still empty, then we are done!
            auto *nextStr = reinterpret_cast<decltype(localize_get_hook) *>(localize_get_orig)(
                    i + 1);
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
    load_assets = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *thisObj, Il2CppString *name,
                                                     const Il2CppType *type)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.AssetBundleModule.dll", "UnityEngine",
                    "AssetBundle", "LoadAsset", 2)
    );

    uobject_get_name = reinterpret_cast<Il2CppString *(*)(Il2CppObject *uObject)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.CoreModule.dll", "UnityEngine",
                    "Object", "GetName", -1)
    );

    auto populate_with_errors_addr = il2cpp_symbols::get_method_pointer(
            "UnityEngine.TextRenderingModule.dll",
            "UnityEngine", "TextGenerator",
            "PopulateWithErrors", 3
    );

    auto get_preferred_width_addr = il2cpp_symbols::get_method_pointer(
            "UnityEngine.TextRenderingModule.dll",
            "UnityEngine", "TextGenerator",
            "GetPreferredWidth", 2
    );

    auto localizeextension_text_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop", "LocalizeExtention", "Text", 1
    );

    // have to do this way because there's Get(TextId id) and Get(string id)
    // the string one looks like will not be called by elsewhere
    auto localize_get_addr = il2cpp_symbols::find_method("umamusume.dll", "Gallop", "Localize",
                                                         [](const MethodInfo *method) {
                                                             return method->name == "Get"s &&
                                                                    method->parameters->parameter_type->type ==
                                                                    IL2CPP_TYPE_VALUETYPE;
                                                         });

    auto update_addr = il2cpp_symbols::get_method_pointer(
            "DOTween.dll", "DG.Tweening.Core", "TweenManager", "Update", 3
    );

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

    auto story_timeline_controller_play_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "StoryTimelineController", "Play", 0);

    auto story_race_textasset_load_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "StoryRaceTextAsset", "Load", 0);

    auto get_modified_string_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "GallopUtil", "GetModifiedString", -1);

    auto on_populate_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "OnPopulateMesh", 1
    );

    auto textcommon_awake_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "Awake", 0
    );

    text_get_text = reinterpret_cast<Il2CppString *(*)(void *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "get_text", 0
            )
    );
    text_set_text = reinterpret_cast<void (*)(void *, Il2CppString *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_text", 1
            )
    );

    text_assign_font = reinterpret_cast<void (*)(void *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "AssignDefaultFont", 0
            )
    );

    text_set_font = reinterpret_cast<void (*)(void *, Il2CppObject *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_font", 1
            )
    );

    text_get_font = reinterpret_cast<Il2CppObject *(*)(void *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "get_font", 0
            )
    );

    text_get_size = reinterpret_cast<int (*)(void *)>(
            il2cpp_symbols::get_method_pointer(
                    "umamusume.dll", "Gallop",
                    "TextCommon", "get_FontSize", 0
            )
    );

    text_set_size = reinterpret_cast<void (*)(void *, int)>(
            il2cpp_symbols::get_method_pointer(
                    "umamusume.dll", "Gallop",
                    "TextCommon", "set_FontSize", 1
            )
    );

    text_get_linespacing = reinterpret_cast<float (*)(void *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "get_lineSpacing", 0
            )
    );

    text_set_style = reinterpret_cast<void (*)(void *, int)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_fontStyle", 1
            )
    );

    text_set_linespacing = reinterpret_cast<void (*)(void *, float)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_lineSpacing", 1
            )
    );

    auto set_fps_addr =
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.CoreModule.dll", "UnityEngine",
                    "Application", "set_targetFrameRate", 1);

    auto an_text_fix_data_addr = reinterpret_cast<void (*)(
            Il2CppObject *thisObj)>(il2cpp_symbols::get_method_pointer("Plugins.dll",
                                                                       "AnimateToUnity", "AnText",
                                                                       "_FixData", 0));

    auto an_text_set_material_to_textmesh_addr = reinterpret_cast<void (*)(
            Il2CppObject *thisObj)>(il2cpp_symbols::get_method_pointer("Plugins.dll",
                                                                       "AnimateToUnity", "AnText",
                                                                       "_SetMaterialToTextMesh",
                                                                       0));

    auto load_zekken_composite_resource_addr = reinterpret_cast<void (*)(
            Il2CppObject *thisObj)>(il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop",
                                                                       "ModelLoader",
                                                                       "LoadZekkenCompositeResourceInternal",
                                                                       0));

    auto wait_resize_ui_addr = reinterpret_cast<void (*)(Il2CppObject *thisObj, bool isPortrait,
                                                         bool isShowOrientationGuide)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop", "UIManager", "WaitResizeUI", 2));

    auto load_from_file = reinterpret_cast<Il2CppObject *(*)(
            Il2CppString *path)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle", "LoadFromFile", 1));

    /*auto load_from_memory_async = reinterpret_cast<Il2CppObject *(*)(
            Il2CppArray *bytes)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle",
            "LoadFromMemoryAsync",
            1));*/

    if (!assets) {
        auto assetbundlePath = localify::u8_u16(g_font_assetbundle_path);
        if (!assetbundlePath.starts_with(u"/")) {
            assetbundlePath.insert(0, u16string(u"/sdcard/Android/data/").append(
                    localify::u8_u16(GetCurrentPackageName())).append(u"/"));
        }
        assets = load_from_file(
                il2cpp_string_new_utf16(assetbundlePath.data(), assetbundlePath.length()));

        if (!assets && filesystem::exists(assetbundlePath)) {
            LOGW("Asset founded but not loaded. Maybe Asset BuildTarget is not for Android");
        }

        /* Load from Memory Async

         std::ifstream infile(localify::u16_u8(assetbundlePath).data(), std::ios_base::binary);

        std::vector<char> buffer((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());

        Il2CppArray* assetBytes = il2cpp_array_new(il2cpp_defaults.byte_class, buffer.size());

        for (int i = 0; i < buffer.size(); i++) {
            il2cpp_array_set(assetBytes, char, i, buffer[i]);
        }
        Il2CppObject* createReq = load_from_memory_async(assetBytes);

        auto get_assetBundle = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject* thisObj)>(il2cpp_symbols::get_method_pointer(
                "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundleCreateRequest", "get_assetBundle",
                0));
        auto get_isDone = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject* thisObj)>(il2cpp_symbols::get_method_pointer(
                "UnityEngine.CoreModule.dll", "UnityEngine", "AsyncOperation", "get_isDone",
                0));

        std::thread load_thread([createReq, get_assetBundle, get_isDone]() {
            while (!get_isDone(createReq)) {}
            assets = get_assetBundle(createReq);
        });
        load_thread.detach();*/
    }

    if (!assets) {
        LOGI("Asset not loaded.");
    } else {
        LOGI("Asset loaded: %p", assets);
    }

#define ADD_HOOK(_name_) \
    DobbyHook((void *)_name_##_addr, (void *) _name_##_hook, (void **) &_name_##_orig);
#pragma endregion

    ADD_HOOK(get_preferred_width)

    ADD_HOOK(an_text_fix_data)

    ADD_HOOK(an_text_set_material_to_textmesh)

    ADD_HOOK(load_zekken_composite_resource)

    ADD_HOOK(wait_resize_ui)

    // hook UnityEngine.TextGenerator::PopulateWithErrors to modify text
    ADD_HOOK(populate_with_errors)

    ADD_HOOK(localizeextension_text)

    // Looks like they store all localized texts that used by code in a dict
    ADD_HOOK(localize_get)

    ADD_HOOK(story_timeline_controller_play)

    ADD_HOOK(story_race_textasset_load)

    ADD_HOOK(get_modified_string)

    ADD_HOOK(update)

    ADD_HOOK(query_ctor)
    ADD_HOOK(query_getstr)
    ADD_HOOK(query_dispose)

    if (g_replace_to_builtin_font || g_replace_to_custom_font) {
        ADD_HOOK(on_populate)
        ADD_HOOK(textcommon_awake)
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
