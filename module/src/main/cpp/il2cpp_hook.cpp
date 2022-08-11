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

Il2CppObject *(*load_from_file)(Il2CppString *path);

Il2CppObject *(*load_assets)(Il2CppObject *thisObj, Il2CppString *name, const Il2CppType *type);

Il2CppString *(*uobject_get_name)(Il2CppObject *uObject);

Il2CppString *(*get_unityVersion)();

Il2CppObject *sceneManager = nullptr;

const Il2CppType *
GetRuntimeType(const char *assemblyName, const char *namespaze, const char *klassName) {
    auto dummyObj = (Il2CppObject *) il2cpp_object_new(
            il2cpp_symbols::get_class(assemblyName, namespaze, klassName));
    auto (*get_type)(Il2CppObject *thisObj) = reinterpret_cast<const Il2CppType *(*)(
            Il2CppObject *thisObj)>(il2cpp_symbols::get_method_pointer("mscorlib.dll", "System",
                                                                       "Object", "GetType", 0));
    return get_type(dummyObj);
}

Boolean GetBoolean(bool value) {
    return reinterpret_cast<Boolean(*)(Il2CppString *value)>(il2cpp_symbols::get_method_pointer(
            "mscorlib.dll", "System", "Boolean", "Parse", 1))(
            il2cpp_string_new(value ? "true" : "false"));
}

Il2CppObject *GetCustomFont() {
    if (!assets) return nullptr;
    return load_assets(assets, il2cpp_string_new(g_font_asset_name.data()),
                       GetRuntimeType("UnityEngine.TextRenderingModule.dll", "UnityEngine",
                                      "Font"));
}

string GetUnityVersion() {
    string version(localify::u16_u8(get_unityVersion()->start_char));
    return version;
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
    Il2CppString *localized = localify::get_localized_string(id);
    return localized ? localized
                     : reinterpret_cast<decltype(localizeextension_text_hook) *>(localizeextension_text_orig)(
                    id
            );
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
    if (g_force_landscape) {
        isPortrait = false;
        isShowOrientationGuide = false;
    }
    if (!g_ui_loading_show_orientation_guide) {
        isShowOrientationGuide = false;
    }
    return reinterpret_cast<decltype(wait_resize_ui_hook) *>(wait_resize_ui_orig)(
            thisObj, isPortrait, isShowOrientationGuide);
}

void *set_anti_aliasing_orig = nullptr;

void set_anti_aliasing_hook(int level) {
    reinterpret_cast<decltype(set_anti_aliasing_hook) *>(set_anti_aliasing_orig)(g_anti_aliasing);
}

Il2CppObject *display_main = nullptr;

Il2CppObject *(*display_get_main)();

int (*get_system_width)(Il2CppObject *thisObj);

int (*get_system_height)(Il2CppObject *thisObj);

void *set_resolution_orig = nullptr;

void set_resolution_hook(int width, int height, bool fullscreen) {
    int systemWidth = get_system_width(display_main);
    int systemHeight = get_system_height(display_main);
    // Unity 2019 not invert width, height on landscape
    if ((width > height && systemWidth < systemHeight) || g_force_landscape) {
        reinterpret_cast<decltype(set_resolution_hook) *>(set_resolution_orig)(
                systemHeight, systemWidth,
                fullscreen);
        return;
    }
    reinterpret_cast<decltype(set_resolution_hook) *>(set_resolution_orig)(
            systemWidth, systemHeight,
            fullscreen);
}

void *apply_graphics_quality_orig = nullptr;

void apply_graphics_quality_hook(Il2CppObject *thisObj, int quality, bool force) {
    reinterpret_cast<decltype(apply_graphics_quality_hook) *>(apply_graphics_quality_orig)(thisObj,
                                                                                           g_graphics_quality,
                                                                                           true);
}

void *load_one_orig = nullptr;

Boolean load_one_hook(Il2CppObject *thisObj, Il2CppObject *handle, Il2CppObject *request) {
    FieldInfo *hNameField = il2cpp_class_get_field_from_name(request->klass, "hname");
    Il2CppString *hName = nullptr;
    il2cpp_field_get_value(request, hNameField, &hName);
    auto hNameStr = localify::u16_u8(hName->start_char);

    if (g_replace_assets.find(hNameStr) != g_replace_assets.end()) {
        auto &replaceAsset = g_replace_assets.at(hNameStr);
        auto set_assetBundle = reinterpret_cast<void (*)(
                Il2CppObject *thisObj,
                Il2CppObject *assetBundle)>(il2cpp_symbols::get_method_pointer(
                "_Cyan.dll", "Cyan.Loader", "AssetHandle", "SetAssetBundle",
                1));

        auto get_IsLoaded = reinterpret_cast<Boolean(*)(
                Il2CppObject *thisObj)>(il2cpp_symbols::get_method_pointer(
                "_Cyan.dll", "Cyan.Loader", "AssetHandle", "get_IsLoaded",
                0));

        if (!replaceAsset.asset) {
            LOGD("%s", replaceAsset.path.data());
            replaceAsset.asset = load_from_file(il2cpp_string_new(replaceAsset.path.data()));
        }
        set_assetBundle(handle, replaceAsset.asset);
        return get_IsLoaded(handle);

    }
    return reinterpret_cast<decltype(load_one_hook) *>(load_one_orig)(thisObj, handle, request);
}

void *assetbundle_unload_orig = nullptr;

void assetbundle_unload_hook(Il2CppObject *thisObj, Boolean unloadAllLoadedObjects) {
    for (auto &pair: g_replace_assets) {
        if (pair.second.asset == thisObj) {
            pair.second.asset = nullptr;
        }
    }
    reinterpret_cast<decltype(assetbundle_unload_hook) *>(assetbundle_unload_orig)(thisObj,
                                                                                   unloadAllLoadedObjects);
}

void *ChangeScreenOrientation_orig = nullptr;

Il2CppObject *ChangeScreenOrientation_hook(ScreenOrientation targetOrientation, bool isForce) {
    return reinterpret_cast<decltype(ChangeScreenOrientation_hook) *>(ChangeScreenOrientation_orig)(
            g_force_landscape ? ScreenOrientation::Landscape : targetOrientation, isForce);
}

void *CanvasScaler_set_referenceResolution_orig = nullptr;

void CanvasScaler_set_referenceResolution_hook(Il2CppObject *thisObj, Vector2_t res) {
    if (g_force_landscape) {
        res.x = res.x * g_force_landscape_ui_scale;
        res.y = res.y * g_force_landscape_ui_scale;
    }
    return reinterpret_cast<decltype(CanvasScaler_set_referenceResolution_hook) *>(CanvasScaler_set_referenceResolution_orig)(
            thisObj, res);
}

void *SetResolution_orig = nullptr;

void SetResolution_hook(int w, int h, bool fullscreen, bool forceUpdate) {
    if (sceneManager) {
        SceneId sceneId = reinterpret_cast<SceneId (*)(
                Il2CppObject *)>(il2cpp_class_get_method_from_name(sceneManager->klass,
                                                                   "GetCurrentSceneId",
                                                                   0)->methodPointer)(sceneManager);
        if (sceneId == SceneId::Live) {
            return;
        }
        if (sceneId == SceneId::Story) {
            return;
        }
        if (sceneId == SceneId::Episode) {
            return;
        }
    }

    reinterpret_cast<decltype(SetResolution_hook) *>(SetResolution_orig)(w, h, fullscreen,
                                                                         forceUpdate);
    if (g_force_landscape) {
        reinterpret_cast<decltype(set_resolution_hook) *>(set_resolution_orig)(h, w, fullscreen);
    }
};

void *Screen_set_orientation_orig = nullptr;

void Screen_set_orientation_hook(ScreenOrientation orientation) {
    if ((orientation == ScreenOrientation::Portrait ||
         orientation == ScreenOrientation::PortraitUpsideDown) && g_force_landscape) {
        orientation = ScreenOrientation::Landscape;
    }
    reinterpret_cast<decltype(Screen_set_orientation_hook) *>(Screen_set_orientation_orig)(
            orientation);
}

void *DeviceOrientationGuide_Show_orig = nullptr;

void DeviceOrientationGuide_Show_hook(Il2CppObject *thisObj, bool isTargetOrientationPortrait,
                                      int target) {
    reinterpret_cast<decltype(DeviceOrientationGuide_Show_hook) *>(DeviceOrientationGuide_Show_orig)(
            thisObj,
            !g_force_landscape && isTargetOrientationPortrait, g_force_landscape ? 2 : target);
}

void *NowLoading_Show_orig = nullptr;

void NowLoading_Show_hook(Il2CppObject *thisObj, int type, Il2CppObject *onComplete,
                          float overrideDuration) {
    // NowLoadingOrientation
    if (type == 2 && (g_force_landscape || !g_ui_loading_show_orientation_guide)) {
        // NowLoadingTips
        type = 0;
    }
    reinterpret_cast<decltype(NowLoading_Show_hook) *>(NowLoading_Show_orig)(
            thisObj,
            type,
            onComplete, overrideDuration);
}

void *WaitDeviceOrientation_orig = nullptr;

void WaitDeviceOrientation_hook(ScreenOrientation targetOrientation) {
    if ((targetOrientation == ScreenOrientation::Portrait ||
         targetOrientation == ScreenOrientation::PortraitUpsideDown) && g_force_landscape) {
        targetOrientation = ScreenOrientation::Landscape;
    }
    reinterpret_cast<decltype(WaitDeviceOrientation_hook) *>(WaitDeviceOrientation_orig)(
            targetOrientation);
}

void *SceneManager_ctor_orig = nullptr;

void SceneManager_ctor_hook(Il2CppObject *thisObj) {
    sceneManager = thisObj;
    reinterpret_cast<decltype(SceneManager_ctor_hook) *>(SceneManager_ctor_orig)(thisObj);
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

    get_unityVersion = reinterpret_cast<Il2CppString *(*)()>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.CoreModule.dll", "UnityEngine",
                    "Application", "get_unityVersion", -1)
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

    auto set_anti_aliasing_addr = reinterpret_cast<void (*)(
            int)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine",
                                                     "QualitySettings", "set_antiAliasing", 1));

    display_get_main = reinterpret_cast<Il2CppObject *(*)()>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.CoreModule.dll",
            "UnityEngine",
            "Display", "get_main", -1));

    display_main = display_get_main();

    get_system_width = reinterpret_cast<int (*)(Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.CoreModule.dll",
            "UnityEngine",
            "Display", "get_systemWidth", 0));

    get_system_height = reinterpret_cast<int (*)(
            Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.CoreModule.dll",
            "UnityEngine",
            "Display", "get_systemHeight", 0));

    auto set_resolution_addr = reinterpret_cast<void (*)(
            int)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine",
                                                     "Screen", "SetResolution", 3));

    auto apply_graphics_quality_addr = reinterpret_cast<void (*)(
            Il2CppObject *, Il2CppObject *, bool)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop",
            "GraphicSettings", "ApplyGraphicsQuality", 2));

    auto ChangeScreenOrientation_addr = reinterpret_cast<void (*)(
            ScreenOrientation, bool)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop",
            "Screen", "ChangeScreenOrientation", 2));

    auto Screen_set_orientation_addr = reinterpret_cast<void (*)(
            ScreenOrientation)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.CoreModule.dll",
            "UnityEngine",
            "Screen", "set_orientation", 1));

    auto SetResolution_addr = reinterpret_cast<void (*)(int, int, bool,
                                                        bool)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "Screen", "SetResolution", 4));

    auto SceneManager_ctor_addr = reinterpret_cast<void (*)(
            Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "SceneManager", ".ctor", 0));

    auto DeviceOrientationGuide_Show_addr = reinterpret_cast<void (*)(bool,
                                                                      int)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "DeviceOrientationGuide", "Show", 2));

    auto NowLoading_Show_addr = reinterpret_cast<void (*)(int, Il2CppObject *,
                                                          float)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "NowLoading", "Show", 3));

    auto WaitDeviceOrientation_addr = reinterpret_cast<void (*)(
            ScreenOrientation)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "Screen", "WaitDeviceOrientation", 1));

    auto CanvasScaler_set_referenceResolution_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                                               float)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.UI.dll",
            "UnityEngine.UI", "CanvasScaler", "set_referenceResolution", 1));

    load_from_file = reinterpret_cast<Il2CppObject *(*)(
            Il2CppString *path)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle", "LoadFromFile", 1));

    /*auto load_from_memory_async = reinterpret_cast<Il2CppObject *(*)(
            Il2CppArray *bytes)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle",
            "LoadFromMemoryAsync",
            1));*/

    auto assetbundle_unload_addr = reinterpret_cast<Il2CppObject *(*)(
            Il2CppString *path)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle", "Unload", 1));

    auto load_one_addr = reinterpret_cast<Boolean(*)(Il2CppObject *thisObj, Il2CppObject *handle,
                                                     Il2CppObject *request)>(il2cpp_symbols::get_method_pointer(
            "_Cyan.dll", "Cyan.Loader", "AssetLoader", "LoadOne", 2));

    if (!assets && !g_font_assetbundle_path.empty()) {
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
    LOGD("ADD_HOOK: %s", #_name_); \
    DobbyHook((void *)_name_##_addr, (void *) _name_##_hook, (void **) &_name_##_orig);
#pragma endregion
    ADD_HOOK(SceneManager_ctor);

    ADD_HOOK(CanvasScaler_set_referenceResolution);

    ADD_HOOK(Screen_set_orientation);

    ADD_HOOK(SetResolution);

    ADD_HOOK(WaitDeviceOrientation);

    ADD_HOOK(NowLoading_Show);

    ADD_HOOK(DeviceOrientationGuide_Show);

    ADD_HOOK(ChangeScreenOrientation);

    ADD_HOOK(assetbundle_unload);

    ADD_HOOK(load_one);

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

    if (g_ui_use_system_resolution) {
        ADD_HOOK(set_resolution);
    }

    if (g_graphics_quality != -1) {
        ADD_HOOK(apply_graphics_quality);
    }

    if (g_anti_aliasing != -1) {
        ADD_HOOK(set_anti_aliasing);
    }

    LOGI("Unity Version: %s", GetUnityVersion().data());
}

void il2cpp_hook_init(void *handle) {
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
}

string get_application_version() {
    reinterpret_cast<void (*)()>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.AndroidJNIModule.dll",
                    "UnityEngine",
                    "AndroidJNI",
                    "AttachCurrentThread",
                    -1))();
    auto version = string(localify::u16_u8(
            reinterpret_cast<Il2CppString *(*)()>(
                    il2cpp_symbols::get_method_pointer(
                            "umamusume.dll", "Gallop",
                            "DeviceHelper", "GetAppVersionName",
                            -1))()->start_char));
    return version;
}

void il2cpp_hook() {
    hookMethods();
}
