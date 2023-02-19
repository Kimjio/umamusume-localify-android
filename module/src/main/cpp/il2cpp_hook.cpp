#include "stdinclude.hpp"
#include "il2cpp_hook.h"
#include "il2cpp/il2cpp_symbols.h"
#include "localify/localify.h"
#include "logger/logger.h"
#include <codecvt>
#include <thread>
#include <rapidjson/rapidjson.h>
#include <rapidjson/prettywriter.h>
#include <sstream>
#include <regex>
#include <SQLiteCpp/SQLiteCpp.h>

using namespace il2cpp_symbols;
using namespace localify;
using namespace logger;

string GotoTitleError =
        "내부적으로 오류가 발생하여 홈으로 이동합니다.\n\n"
        "경우에 따라서 <color=#ff911c><i>타이틀</i></color>로 돌아가거나, \n"
        "게임 <color=#ff911c><i>다시 시작</i></color>이 필요할 수 있습니다.";

string GotoTitleErrorJa =
        "内部的にエラーが発生し、ホームに移動します。\n\n"
        "場合によっては、<color=#ff911c><i>タイトル</i></color>に戻るか、\n"
        "ゲーム<color=#ff911c><i>再起動</i></color>が必要になる場合がありますあります。";

string GotoTitleErrorHan =
        "內部發生錯誤，移動到主頁。\n\n"
        "在某些情況下，可能需要返回<color=#ff911c><i>標題</i></color>に戻るか，\n"
        "或者遊戲<color=#ff911c><i>重新開始</i></color>。";

static void *il2cpp_handle = nullptr;
static uint64_t il2cpp_base = 0;

bool resolutionIsSet = false;

Il2CppObject *assets = nullptr;

Il2CppObject *replaceAssets = nullptr;

Il2CppObject *(*load_from_file)(Il2CppString *path);

Il2CppObject *(*load_assets)(Il2CppObject *thisObj, Il2CppString *name, Il2CppObject *type);

Il2CppArray *(*get_all_asset_names)(Il2CppObject *thisObj);

Il2CppString *(*uobject_get_name)(Il2CppObject *uObject);

bool (*uobject_IsNativeObjectAlive)(Il2CppObject *uObject);

Il2CppString *(*get_unityVersion)();

DateTime (*FromUnixTimeToLocaleTime)(long unixTime);

void *(*Array_GetValue)(Il2CppArray *thisObj, long index);

Il2CppObject *sceneManager = nullptr;

Il2CppObject *uiManager = nullptr;

vector<string> replaceAssetNames;

Il2CppObject *masterDataManager = nullptr;

Il2CppObject *
GetRuntimeType(const char *assemblyName, const char *namespaze, const char *klassName) {
    auto dummyObj = (Il2CppObject *) il2cpp_object_new(
            il2cpp_symbols::get_class(assemblyName, namespaze, klassName));
    auto (*get_type)(Il2CppObject *thisObj) = reinterpret_cast<Il2CppObject *(*)(
            Il2CppObject *thisObj)>(il2cpp_symbols::get_method_pointer("mscorlib.dll", "System",
                                                                       "Object", "GetType", 0));
    return get_type(dummyObj);
}

Boolean GetBoolean(bool value) {
    return reinterpret_cast<Boolean (*)(Il2CppString *value)>(il2cpp_symbols::get_method_pointer(
            "mscorlib.dll", "System", "Boolean", "Parse", 1))(
            il2cpp_string_new(value ? "true" : "false"));
}

Int32Object *GetInt32Instance(int value) {
    return reinterpret_cast<Int32Object *>(il2cpp_value_box(
            il2cpp_symbols::get_class("mscorlib.dll", "System", "Int32"), &value));
}

Il2CppObject *ParseEnum(Il2CppObject *runtimeType, const string &name) {
    return reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                              Il2CppString *)>(il2cpp_symbols::get_method_pointer(
            "mscorlib.dll", "System", "Enum", "Parse", 2))(runtimeType,
                                                           il2cpp_string_new(name.data()));
}

Il2CppString *GetEnumName(Il2CppObject *runtimeType, int id) {
    return reinterpret_cast<Il2CppString *(*)(Il2CppObject *,
                                              Int32Object *)>(il2cpp_symbols::get_method_pointer(
            "mscorlib.dll", "System", "Enum", "GetName", 2))(runtimeType, GetInt32Instance(id));
}

unsigned long GetEnumValue(Il2CppObject *runtimeEnum) {
    return reinterpret_cast<unsigned long (*)(Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
            "mscorlib.dll", "System", "Enum", "ToUInt64", 1))(runtimeEnum);
}

unsigned long GetTextIdByName(const string &name) {
    return GetEnumValue(ParseEnum(GetRuntimeType("umamusume.dll", "Gallop", "TextId"), name));
}

string GetTextIdNameById(int id) {
    return localify::u16_u8(
            GetEnumName(GetRuntimeType("umamusume.dll", "Gallop", "TextId"), id)->start_char);
}

Il2CppObject *GetCustomFont() {
    if (!assets) return nullptr;
    if (!g_font_asset_name.empty()) {
        return load_assets(assets, il2cpp_string_new(g_font_asset_name.data()),
                           GetRuntimeType("UnityEngine.TextRenderingModule.dll", "UnityEngine",
                                          "Font"));
    }
    return nullptr;
}

// Fallback not support outline style
Il2CppObject *GetCustomTMPFontFallback() {
    if (!assets) return nullptr;
    auto font = GetCustomFont();
    if (font) {
        return reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *font, int samplingPointSize, int atlasPadding, int renderMode,
                int atlasWidth, int atlasHeight, int atlasPopulationMode,
                bool enableMultiAtlasSupport
        )>
        (
                il2cpp_symbols::get_method_pointer("Unity.TextMeshPro.dll", "TMPro",
                                                   "TMP_FontAsset", "CreateFontAsset", 1)
        )(font, 36, 4, 4165, 8192, 8192, 1, false);
    }
    return nullptr;
}

Il2CppObject *GetCustomTMPFont() {
    if (!assets) return nullptr;
    if (!g_tmpro_font_asset_name.empty()) {
        auto tmpFont = load_assets(assets, il2cpp_string_new(g_tmpro_font_asset_name.data()),
                                   GetRuntimeType("Unity.TextMeshPro.dll", "TMPro",
                                                  "TMP_FontAsset"));
        return tmpFont ? tmpFont : GetCustomTMPFontFallback();
    }
    return GetCustomTMPFontFallback();
}

bool ExecuteCoroutine(Il2CppObject *enumerator) {
    auto executor = il2cpp_object_new(
            il2cpp_symbols::get_class("umamusume.dll", "Gallop", "CoroutineExecutor"));
    reinterpret_cast<void (*)(Il2CppObject *, Il2CppObject *)>(
            il2cpp_class_get_method_from_name(executor->klass, ".ctor", 1)->methodPointer
    )(executor, enumerator);
    return reinterpret_cast<bool (*)(Il2CppObject *)>(
            il2cpp_class_get_method_from_name(executor->klass, "UpdateCoroutine", 0)->methodPointer
    )(executor);
}

string GetUnityVersion() {
    return string(localify::u16_u8(get_unityVersion()->start_char));
}

/**
 * Int64 값을 안전하게 가져오기
 * <p>
 * Int64에 직접 접근 시 포인터 주소가 잘못된 경우, 쓰레기 값이 나올 수 있음.
 * Int64를 Il2Cpp 문자열로 변환 후 파싱하여 사용한다.
 *
 * @param int64Ptr Int64의 주소 값
 * @return Int64의 원시 값
 */
long GetInt64Safety(Int64 *int64Ptr) {
    auto str = reinterpret_cast<Il2CppString *(*)(Int64 *)>(il2cpp_symbols::get_method_pointer(
            "mscorlib.dll", "System", "Int64", "ToString", 0))(int64Ptr);
    return stol(localify::u16_u8(str->start_char));
}

void *populate_with_errors_orig = nullptr;

bool populate_with_errors_hook(void *thisObj, Il2CppString *str, TextGenerationSettings_t *settings,
                               void *context) {
    return reinterpret_cast<decltype(populate_with_errors_hook) * > (populate_with_errors_orig)(
            thisObj, localify::get_localized_string(str), settings, context
    );
}


void *localizeextension_text_orig = nullptr;

Il2CppString *localizeextension_text_hook(int id) {
    auto orig_result = reinterpret_cast<decltype(localizeextension_text_hook) *>(localizeextension_text_orig)(
            id);
    auto result = g_static_entries_use_text_id_name ?
                  localify::get_localized_string(GetTextIdNameById(id)) :
                  g_static_entries_use_hash ?
                  localify::get_localized_string(orig_result) : localify::get_localized_string(id);
    return result ? result : orig_result;
}

void *get_preferred_width_orig = nullptr;

float
get_preferred_width_hook(void *thisObj, Il2CppString *str, TextGenerationSettings_t *settings) {
    return reinterpret_cast<decltype(get_preferred_width_hook) * > (get_preferred_width_orig)(
            thisObj, localify::get_localized_string(str), settings
    );
}

void *localize_get_orig = nullptr;

Il2CppString *localize_get_hook(int id) {
    auto orig_result = reinterpret_cast<decltype(localize_get_hook) * > (localize_get_orig)(id);
    auto result = g_static_entries_use_text_id_name ?
                  localify::get_localized_string(GetTextIdNameById(id)) :
                  g_static_entries_use_hash ?
                  localify::get_localized_string(orig_result) : localify::get_localized_string(id);

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
    reinterpret_cast<decltype(an_text_set_material_to_textmesh_hook) * >
    (an_text_set_material_to_textmesh_orig)(
            thisObj);
    if (!(assets && g_replace_to_custom_font)) return;

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
    reinterpret_cast<decltype(an_text_fix_data_hook) * > (an_text_fix_data_orig)(thisObj);
    FieldInfo *field = il2cpp_class_get_field_from_name(thisObj->klass, "_text");
    Il2CppString *str;
    il2cpp_field_get_value(thisObj, field, &str);
    il2cpp_field_set_value(thisObj, field, localify::get_localized_string(str));
}

void *update_orig = nullptr;

void *update_hook(Il2CppObject *thisObj, void *updateType, float deltaTime, float independentTime) {
    return reinterpret_cast<decltype(update_hook) * > (update_orig)(thisObj, updateType,
                                                                    deltaTime *
                                                                    g_ui_animation_scale,
                                                                    independentTime *
                                                                    g_ui_animation_scale);
}

unordered_map<void *, SQLite::Statement *> text_queries;
unordered_map<void *, bool> replacement_queries_can_next;

SQLite::Database *replacementMDB;

void *query_setup_orig = nullptr;


void query_setup_hook(Il2CppObject *thisObj, void *conn, Il2CppString *sql) {
    reinterpret_cast<decltype(query_setup_hook) *>(query_setup_orig)(thisObj, conn, sql);

    auto sqlQuery = u16string(sql->start_char);

    if (sqlQuery.find(u"text_data") != string::npos ||
        sqlQuery.find(u"character_system_text") != string::npos ||
        sqlQuery.find(u"race_jikkyo_comment") != string::npos ||
        sqlQuery.find(u"race_jikkyo_message") != string::npos) {
        auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
        intptr_t *stmtPtr;
        il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
        try {
            if (replacementMDB) {
                text_queries.emplace(stmtPtr,
                                     new SQLite::Statement(*replacementMDB,
                                                           localify::u16_u8(sqlQuery)));
            } else {
                text_queries.emplace(stmtPtr, nullptr);
            }
        } catch (exception &e) {
            LOGD("query_setup ERROR: %s", e.what());
        }
    }
}


void *Plugin_sqlite3_step_orig = nullptr;

bool Plugin_sqlite3_step_hook(intptr_t *pStmt) {
    LOGD("Plugin_sqlite3_step_hook");
    if (text_queries.contains(pStmt)) {
        try {
            auto stmt = text_queries.at(pStmt);
            if (stmt) {
                if (stmt->getQuery().find("`race_jikkyo_message`;") != string::npos ||
                    stmt->getQuery().find("`race_jikkyo_comment`;") != string::npos) {
                    if (replacement_queries_can_next.find(pStmt) ==
                        replacement_queries_can_next.end()) {
                        replacement_queries_can_next.emplace(pStmt, true);
                    }
                    if (replacement_queries_can_next.at(pStmt)) {
                        try {
                            stmt->executeStep();
                        } catch (exception &e) {
                        }
                    }
                } else {
                    stmt->executeStep();
                }
            }
        } catch (exception &e) {
        }
    }

    return reinterpret_cast<decltype(Plugin_sqlite3_step_hook) *>(Plugin_sqlite3_step_orig)(pStmt);
}

void *Plugin_sqlite3_reset_orig = nullptr;

bool Plugin_sqlite3_reset_hook(intptr_t *pStmt) {
    if (text_queries.contains(pStmt)) {
        try {
            auto stmt = text_queries.at(pStmt);
            if (stmt) {
                stmt->reset();
                stmt->clearBindings();
                replacement_queries_can_next.insert_or_assign(pStmt, true);
            }
        }
        catch (exception &e) {
        }
    }
    return reinterpret_cast<decltype(Plugin_sqlite3_reset_hook) *>(Plugin_sqlite3_reset_orig)(
            pStmt);
}

void *query_step_orig = nullptr;

bool query_step_hook(Il2CppObject *thisObj) {
    auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
    if (text_queries.contains(stmtPtr)) {
        try {
            auto stmt = text_queries.at(stmtPtr);
            if (stmt) {
                stmt->executeStep();
            }
        }
        catch (exception &e) {
        }
    }
    return reinterpret_cast<decltype(query_step_hook) *>(query_step_orig)(thisObj);
}

void *prepared_query_reset_orig = nullptr;

bool prepared_query_reset_hook(Il2CppObject *thisObj) {
    auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
    if (text_queries.contains(stmtPtr)) {
        try {
            auto stmt = text_queries.at(stmtPtr);
            if (stmt) {

                stmt->reset();
                stmt->clearBindings();
                replacement_queries_can_next.insert_or_assign(stmtPtr, true);
            }
        }
        catch (exception &e) {
        }
    }
    return reinterpret_cast<decltype(prepared_query_reset_hook) *>(prepared_query_reset_orig)(
            thisObj);
}

void *prepared_query_bind_text_orig = nullptr;

bool prepared_query_bind_text_hook(Il2CppObject *thisObj, int idx, Il2CppString *text) {
    auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
    if (text_queries.contains(stmtPtr)) {
        try {
            auto stmt = text_queries.at(stmtPtr);
            if (stmt) {
                stmt->bind(idx, localify::u16_u8(text->start_char));
            }
        }
        catch (exception &e) {
        }
    }
    return reinterpret_cast<decltype(prepared_query_bind_text_hook) *>(prepared_query_bind_text_orig)(
            thisObj, idx, text);
}

void *prepared_query_bind_int_orig = nullptr;

bool prepared_query_bind_int_hook(Il2CppObject *thisObj, int idx, int iValue) {
    auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
    if (text_queries.contains(stmtPtr)) {
        try {
            auto stmt = text_queries.at(stmtPtr);
            if (stmt) {
                stmt->bind(idx, iValue);
            }
        }
        catch (exception &e) {
        }
    }
    return reinterpret_cast<decltype(prepared_query_bind_int_hook) *>(prepared_query_bind_int_orig)(
            thisObj, idx, iValue);
}

void *prepared_query_bind_long_orig = nullptr;

bool prepared_query_bind_long_hook(Il2CppObject *thisObj, int idx, int64_t lValue) {
    auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
    if (text_queries.contains(stmtPtr)) {
        try {
            auto stmt = text_queries.at(stmtPtr);
            if (stmt) {
                stmt->bind(idx, lValue);
            }
        }
        catch (exception &e) {
        }
    }
    return reinterpret_cast<decltype(prepared_query_bind_long_hook) *>(prepared_query_bind_long_orig)(
            thisObj, idx, lValue);
}

void *prepared_query_bind_double_orig = nullptr;

bool prepared_query_bind_double_hook(Il2CppObject *thisObj, int idx, double rValue) {
    auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
    if (text_queries.contains(stmtPtr)) {
        try {
            auto stmt = text_queries.at(stmtPtr);
            if (stmt) {
                stmt->bind(idx, rValue);
            }
        }
        catch (exception &e) {
        }
    }
    return reinterpret_cast<decltype(prepared_query_bind_double_hook) *>(prepared_query_bind_double_orig)(
            thisObj, idx, rValue);
}

void *query_dispose_orig = nullptr;

void query_dispose_hook(Il2CppObject *thisObj) {
    auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
    if (text_queries.contains(stmtPtr))
        text_queries.erase(stmtPtr);

    return reinterpret_cast<decltype(query_dispose_hook) *>(query_dispose_orig)(thisObj);
}

int (*query_getint)(Il2CppObject *thisObj, int index) = nullptr;

void *query_gettext_orig = nullptr;

Il2CppString *query_gettext_hook(Il2CppObject *thisObj, int idx) {
    auto stmtField = il2cpp_class_get_field_from_name(thisObj->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(thisObj, stmtField, &stmtPtr);
    auto result = reinterpret_cast<decltype(query_gettext_hook) *>(query_gettext_orig)(thisObj,
                                                                                       idx);

    if (text_queries.contains(stmtPtr)) {
        try {
            auto stmt = text_queries.at(stmtPtr);
            if (stmt) {
                string text;
                if (stmt->hasRow()) {
                    text = stmt->getColumn(idx).getString();
                    if (!text.empty()) {
                        if (stmt->getQuery().find("`race_jikkyo_message`;") != string::npos ||
                            stmt->getQuery().find("`race_jikkyo_comment`;") != string::npos) {
                            const int id = query_getint(thisObj, 0);
                            const int id1 = stmt->getColumn(0).getInt();
                            const int groupId = query_getint(thisObj, 1);
                            const int groupId1 = stmt->getColumn(1).getInt();
                            if (stmt->hasRow()) {
                                if (id == id1 && groupId == groupId1) {
                                    replacement_queries_can_next.insert_or_assign(stmtPtr, true);
                                    return il2cpp_string_new(text.data());
                                } else {
                                    replacement_queries_can_next.insert_or_assign(stmtPtr, false);
                                }
                            }
                        } else if (stmt->getQuery().find("character_system_text") != string::npos) {
                            int cueId, cueId1;
                            string cueSheet, cueSheet1;
                            if (stmt->getQuery().find("`voice_id`=?") != string::npos) {
                                cueId = query_getint(thisObj, 2);
                                cueId1 = stmt->getColumn(2).getInt();
                                cueSheet = localify::u16_u8(
                                        reinterpret_cast<decltype(query_gettext_hook) *>(query_gettext_orig)(
                                                thisObj, 1)->start_char
                                );
                                cueSheet1 = stmt->getColumn(1).getString();
                            } else {
                                cueId = query_getint(thisObj, 3);
                                cueId1 = stmt->getColumn(3).getInt();
                                cueSheet = localify::u16_u8(
                                        reinterpret_cast<decltype(query_gettext_hook) *>(query_gettext_orig)(
                                                thisObj, 2)->start_char
                                );
                                cueSheet1 = stmt->getColumn(2).getString();
                            }
                            if (cueId == cueId1 && cueSheet == cueSheet1) {
                                return il2cpp_string_new(text.data());
                            }
                        } else {
                            return il2cpp_string_new(text.data());
                        }
                    }
                }
            }
        }
        catch (exception &e) {
        }
        return localify::get_localized_string(result);
    }

    return result;
}

void *MasterCharacterSystemText_CreateOrmByQueryResultWithCharacterId_orig = nullptr;

Il2CppObject *
MasterCharacterSystemText_CreateOrmByQueryResultWithCharacterId_hook(Il2CppObject *thisObj,
                                                                     Il2CppObject *query,
                                                                     int characterId) {
    auto stmtField = il2cpp_class_get_field_from_name(query->klass, "_stmt");
    intptr_t *stmtPtr;
    il2cpp_field_get_value(query, stmtField, &stmtPtr);
    if (text_queries.contains(stmtPtr)) {

        try {
            auto stmt = text_queries.at(stmtPtr);
            if (stmt) {
                if (replacement_queries_can_next.find(stmtPtr) ==
                    replacement_queries_can_next.end()) {
                    replacement_queries_can_next.emplace(stmtPtr, true);
                }
                if (replacement_queries_can_next.at(stmtPtr)) {
                    try {
                        stmt->executeStep();
                    }
                    catch (exception &e) {
                    }
                }
                if (stmt->hasRow()) {
                    const int voiceId = query_getint(query, 0);
                    const int voiceId1 = stmt->getColumn(0).getInt();
                    const int cueId = query_getint(query, 3);
                    const int cueId1 = stmt->getColumn(3).getInt();
                    const string cueSheet = localify::u16_u8(
                            reinterpret_cast<decltype(query_gettext_hook) *>(query_gettext_orig)(
                                    query, 2)->start_char
                    );
                    const string cueSheet1 = stmt->getColumn(2).getString();

                    if (voiceId == voiceId1 && cueId == cueId1 && cueSheet == cueSheet1) {
                        replacement_queries_can_next.insert_or_assign(stmtPtr, true);
                    } else {
                        replacement_queries_can_next.insert_or_assign(stmtPtr, false);
                    }
                }
            }
        }
        catch (exception &e) {
        }
    }
    return reinterpret_cast<decltype(MasterCharacterSystemText_CreateOrmByQueryResultWithCharacterId_hook) *>(
            MasterCharacterSystemText_CreateOrmByQueryResultWithCharacterId_orig
    )(thisObj, query, characterId);
}

void ShowCaptionByNotification(Il2CppObject *audioManager, Il2CppObject *elem) {
    auto characterIdField = il2cpp_class_get_field_from_name(elem->klass, "CharacterId");
    auto voiceIdField = il2cpp_class_get_field_from_name(elem->klass, "VoiceId");
    auto textField = il2cpp_class_get_field_from_name(elem->klass, "Text");
    auto cueSheetField = il2cpp_class_get_field_from_name(elem->klass, "CueSheet");
    auto cueIdField = il2cpp_class_get_field_from_name(elem->klass, "CueId");

    int characterId;
    il2cpp_field_get_value(elem, characterIdField, &characterId);

    int voiceId;
    il2cpp_field_get_value(elem, voiceIdField, &voiceId);

    Il2CppString *text;
    il2cpp_field_get_value(elem, textField, &text);

    Il2CppString *cueSheet;
    il2cpp_field_get_value(elem, cueSheetField, &cueSheet);

    int cueId;
    il2cpp_field_get_value(elem, cueIdField, &cueId);

    auto u8Text = localify::u16_u8(text->start_char);
    replaceAll(u8Text, "\n", " ");
    if (uiManager &&
        u16string(cueSheet->start_char).find(u"_home_") == string::npos &&
        u16string(cueSheet->start_char).find(u"_tc_") == string::npos &&
        u16string(cueSheet->start_char).find(u"_title_") == string::npos &&
        u16string(cueSheet->start_char).find(u"_gacha_") == string::npos &&
        voiceId != 95001 &&
        (characterId < 9000 ||
         voiceId == 70000)) {
        auto ShowNotification = reinterpret_cast<void (*)(Il2CppObject *, Il2CppString *)>(
                il2cpp_class_get_method_from_name(uiManager->klass, "ShowNotification",
                                                  1)->methodPointer
        );
        auto LineHeadWrap = reinterpret_cast<Il2CppString *(*)(Il2CppString *, int)>(
                il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop", "GallopUtil",
                                                   "LineHeadWrap", 2)
        );

        auto notiField = il2cpp_class_get_field_from_name(uiManager->klass, "_notification");
        Il2CppObject *notification;
        il2cpp_field_get_value(uiManager, notiField, &notification);

        auto timeField = il2cpp_class_get_field_from_name(notification->klass, "_displayTime");
        float displayTime;
        il2cpp_field_get_value(notification, timeField, &displayTime);

        float length = reinterpret_cast<float (*)(Il2CppObject *, Il2CppString *, int)>(
                il2cpp_class_get_method_from_name(audioManager->klass, "GetCueLength",
                                                  2)->methodPointer
        )(audioManager, cueSheet, cueId);
        il2cpp_field_set_value(notification, timeField, &length);

        ShowNotification(uiManager, LineHeadWrap(il2cpp_string_new(u8Text.data()), 32));

        il2cpp_field_set_value(notification, timeField, &displayTime);
    }
}

void *AtomSourceEx_SetParameter_orig = nullptr;

void AtomSourceEx_SetParameter_hook(Il2CppObject *thisObj) {
    reinterpret_cast<decltype(AtomSourceEx_SetParameter_hook) *>(AtomSourceEx_SetParameter_orig)(
            thisObj);

    FieldInfo *cueIdField = il2cpp_class_get_field_from_name(thisObj->klass,
                                                             "<CueId>k__BackingField");
    int cueId;
    il2cpp_field_get_value(thisObj, cueIdField, &cueId);

    FieldInfo *cueSheetField = il2cpp_class_get_field_from_name(thisObj->klass, "_cueSheet");
    Il2CppString *cueSheet;
    il2cpp_field_get_value(thisObj, cueSheetField, &cueSheet);

    const regex r(R"((\d{4})(?:\d{2}))");
    smatch stringMatch;
    const auto cueSheetU8 = localify::u16_u8(cueSheet->start_char);
    regex_search(cueSheetU8, stringMatch, r);
    if (!stringMatch.empty()) {
        Il2CppObject *textList = reinterpret_cast<Il2CppObject *(*)(int)>(
                il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop",
                                                   "MasterCharacterSystemText", "GetByCharaId", 1))(
                stoi(stringMatch[1].str()));
        FieldInfo *itemsField = il2cpp_class_get_field_from_name(textList->klass, "_items");
        Il2CppArray *textArr;
        il2cpp_field_get_value(textList, itemsField, &textArr);

        Il2CppObject *audioManager;
        auto klass = il2cpp_symbols::get_class("umamusume.dll", "Gallop", "AudioManager");
        auto instanceField = il2cpp_class_get_field_from_name(klass, "_instance");
        il2cpp_field_static_get_value(instanceField, &audioManager);

        for (int i = 0; i < textArr->max_length; i++) {
            auto elem = reinterpret_cast<Il2CppObject *>(textArr->vector[i]);
            if (elem) {
                auto elemCueIdField = il2cpp_class_get_field_from_name(elem->klass, "CueId");
                auto elemCueSheetField = il2cpp_class_get_field_from_name(elem->klass, "CueSheet");

                Il2CppString *elemCueSheet;
                il2cpp_field_get_value(elem, elemCueSheetField, &elemCueSheet);

                int elemCueId;
                il2cpp_field_get_value(elem, elemCueIdField, &elemCueId);

                if (u16string(elemCueSheet->start_char) == u16string(cueSheet->start_char) &&
                    cueId == elemCueId) {
                    ShowCaptionByNotification(audioManager, elem);
                    return;
                }
            }
        }
    }
}

void *CySpringUpdater_set_SpringUpdateMode_orig = nullptr;

void CySpringUpdater_set_SpringUpdateMode_hook(Il2CppObject *thisObj, int  /*value*/) {
    reinterpret_cast<decltype(CySpringUpdater_set_SpringUpdateMode_hook) *>(CySpringUpdater_set_SpringUpdateMode_orig)(
            thisObj, g_cyspring_update_mode);
}

void *CySpringUpdater_get_SpringUpdateMode_orig = nullptr;

int CySpringUpdater_get_SpringUpdateMode_hook(Il2CppObject *thisObj) {
    CySpringUpdater_set_SpringUpdateMode_hook(thisObj, g_cyspring_update_mode);
    return reinterpret_cast<decltype(CySpringUpdater_get_SpringUpdateMode_hook) *>(CySpringUpdater_get_SpringUpdateMode_orig)(
            thisObj);
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

    return reinterpret_cast<decltype(story_timeline_controller_play_hook) * >
    (story_timeline_controller_play_orig)(
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
    /*auto move_next = reinterpret_cast<bool (*)(
            Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(enumerator->klass, "MoveNext",
                                                                      0)->methodPointer);*/

    while (ExecuteCoroutine(enumerator)) {
        auto key = get_current(enumerator);
        FieldInfo *textField = {il2cpp_class_get_field_from_name(key->klass, "text")};
        Il2CppString *text;
        il2cpp_field_get_value(key, textField, &text);
        il2cpp_field_set_value(key, textField, localify::get_localized_string(text));
    }

    reinterpret_cast<decltype(story_race_textasset_load_hook) * > (story_race_textasset_load_orig)(
            thisObj);
}

void (*text_assign_font)(Il2CppObject *);

void (*text_set_font)(Il2CppObject *, Il2CppObject *);

Il2CppObject *(*text_get_font)(Il2CppObject *);

int (*text_get_size)(Il2CppObject *);

void (*text_set_size)(Il2CppObject *, int);

float (*text_get_linespacing)(Il2CppObject *);

void (*text_set_style)(Il2CppObject *, int);

void (*text_set_linespacing)(Il2CppObject *, float);

Il2CppString *(*text_get_text)(Il2CppObject *);

void (*text_set_text)(Il2CppObject *, Il2CppString *);

void (*text_set_horizontalOverflow)(Il2CppObject *, int);

void (*text_set_verticalOverflow)(Il2CppObject *, int);

int (*textcommon_get_TextId)(Il2CppObject *);

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
    auto textId = textcommon_get_TextId(thisObj);
    if (textId) {
        if (GetTextIdByName("Common0121") == textId ||
            GetTextIdByName("Common0186") == textId ||
            GetTextIdByName("Outgame0028") == textId ||
            GetTextIdByName("Outgame0231") == textId ||
            GetTextIdByName("Character0325") == textId) {
            text_set_horizontalOverflow(thisObj, 1);
            text_set_verticalOverflow(thisObj, 1);
        }
    }
    return reinterpret_cast<decltype(on_populate_hook) * > (on_populate_orig)(thisObj, toFill);
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
    reinterpret_cast<decltype(textcommon_awake_hook) * > (textcommon_awake_orig)(thisObj);
}


void *textcommon_SetTextWithLineHeadWrap_orig = nullptr;

void
textcommon_SetTextWithLineHeadWrap_hook(Il2CppObject *thisObj, Il2CppString *str,
                                        int maxCharacter) {
    reinterpret_cast<decltype(textcommon_SetTextWithLineHeadWrap_hook) *>(textcommon_SetTextWithLineHeadWrap_orig)(
            thisObj, str, maxCharacter * 2);
}

void *textcommon_SetTextWithLineHeadWrapWithColorTag_orig = nullptr;

void textcommon_SetTextWithLineHeadWrapWithColorTag_hook(Il2CppObject *thisObj, Il2CppString *str,
                                                         int maxCharacter) {
    reinterpret_cast<decltype(textcommon_SetTextWithLineHeadWrapWithColorTag_hook) *>(textcommon_SetTextWithLineHeadWrapWithColorTag_orig)(
            thisObj, str, maxCharacter * 2);
}

void *textcommon_SetSystemTextWithLineHeadWrap_orig = nullptr;

void textcommon_SetSystemTextWithLineHeadWrap_hook(Il2CppObject *thisObj, Il2CppObject *systemText,
                                                   int maxCharacter) {
    reinterpret_cast<decltype(textcommon_SetSystemTextWithLineHeadWrap_hook) *>(textcommon_SetSystemTextWithLineHeadWrap_orig)(
            thisObj, systemText, maxCharacter * 2);
}

void *TextMeshProUguiCommon_Awake_orig = nullptr;

void TextMeshProUguiCommon_Awake_hook(Il2CppObject *thisObj) {
    reinterpret_cast<decltype(TextMeshProUguiCommon_Awake_hook) *>(TextMeshProUguiCommon_Awake_orig)(
            thisObj);
    auto customFont = GetCustomTMPFont();
    auto customFontMaterialField = il2cpp_class_get_field_from_name(customFont->klass, "material");
    Il2CppObject *customFontMaterial;
    il2cpp_field_get_value(customFont, customFontMaterialField, &customFontMaterial);

    auto SetFloat = reinterpret_cast<void (*)(Il2CppObject *, Il2CppString *,
                                              float)>(il2cpp_class_get_method_from_name(
            customFontMaterial->klass, "SetFloat", 2)->methodPointer);
    auto SetColor = reinterpret_cast<void (*)(Il2CppObject *, Il2CppString *,
                                              Color_t)>(il2cpp_class_get_method_from_name(
            customFontMaterial->klass, "SetColor", 2)->methodPointer);

    auto origOutlineWidth = reinterpret_cast<float (*)(
            Il2CppObject *)>(il2cpp_class_get_method_from_name(thisObj->klass, "get_outlineWidth",
                                                               0)->methodPointer)(thisObj);

    auto outlineColorDictField = il2cpp_class_get_field_from_name(
            il2cpp_symbols::get_class("umamusume.dll", "Gallop", "ColorPreset"),
            "OutlineColorDictionary");
    Il2CppObject *outlineColorDict;
    il2cpp_field_static_get_value(outlineColorDictField, &outlineColorDict);
    auto colorEnum = reinterpret_cast<int (*)(Il2CppObject *)>(il2cpp_class_get_method_from_name(
            thisObj->klass, "get_OutlineColor", 0)->methodPointer)(thisObj);

    auto entriesField = il2cpp_class_get_field_from_name(outlineColorDict->klass, "entries");
    Il2CppArray *entries;
    il2cpp_field_get_value(outlineColorDict, entriesField, &entries);

    auto colorType = GetRuntimeType("umamusume.dll", "Gallop", "OutlineColorType");

    auto color32 = 0xFFFFFFFF;
    for (int i = 0; i < entries->max_length; i++) {
        auto entry = reinterpret_cast<unsigned long long>(entries->vector[i]);
        auto color = (entry & 0xFFFFFFFF00000000) >> 32;
        auto key = entry & 0xFFFFFFFF;
        if (key == colorEnum && (color != 0xFFFFFFFF && color != 0)) {
            color32 = color;
            break;
        }
        auto enumName = localify::u16_u8(GetEnumName(colorType, colorEnum)->start_char);
        if (enumName == "White"s || enumName == "Black"s) {
            color32 = color;
            break;
        }
    }

    float const a = static_cast<float>((color32 & 0xFF000000) >> 24) / static_cast<float>(0xff);
    float const b = static_cast<float>((color32 & 0xFF0000) >> 16) / static_cast<float>(0xff);
    float const g = static_cast<float>((color32 & 0xFF00) >> 8) / static_cast<float>(0xff);
    float const r = static_cast<float>(color32 & 0xFF) / static_cast<float>(0xff);
    auto origOutlineColor = Color_t{r, g, b, a};

    SetFloat(customFontMaterial, il2cpp_string_new("_OutlineWidth"), origOutlineWidth);
    SetColor(customFontMaterial, il2cpp_string_new("_OutlineColor"), origOutlineColor);

    reinterpret_cast<void (*)(Il2CppObject *, Il2CppObject *)>(il2cpp_class_get_method_from_name(
            thisObj->klass, "set_font", 1)->methodPointer)(thisObj, customFont);
    reinterpret_cast<void (*)(Il2CppObject *, bool)>(il2cpp_class_get_method_from_name(
            thisObj->klass,
            "set_enableWordWrapping",
            1)->methodPointer)(
            thisObj, false);
}

void *get_modified_string_orig = nullptr;

Il2CppString *get_modified_string_hook(Il2CppString *text, Il2CppObject * /*input*/,
                                       bool allowNewLine) {
    if (!allowNewLine) {
        auto u8str = localify::u16_u8(text->start_char);
        replaceAll(u8str, "\n", "");
        return il2cpp_string_new(u8str.data());
    }
    return text;
}

void *set_fps_orig = nullptr;

void set_fps_hook([[maybe_unused]] int value) {
    return reinterpret_cast<decltype(set_fps_hook) * > (set_fps_orig)(g_max_fps);
}

void *load_zekken_composite_resource_orig = nullptr;

void load_zekken_composite_resource_hook(Il2CppObject *thisObj) {
    if ((assets != nullptr) && g_replace_to_custom_font) {
        auto *font = GetCustomFont();
        if (font != nullptr) {
            FieldInfo *zekkenFontField = il2cpp_class_get_field_from_name(thisObj->klass,
                                                                          "_fontZekken");
            il2cpp_field_set_value(thisObj, zekkenFontField, font);
        }
    }
    reinterpret_cast<decltype(load_zekken_composite_resource_hook) * >
    (load_zekken_composite_resource_orig)(
            thisObj);
}

void *UIManager_OnInitialize_orig = nullptr;

void UIManager_OnInitialize_hook(Il2CppObject *thisObj) {
    reinterpret_cast<decltype(UIManager_OnInitialize_hook) *>(UIManager_OnInitialize_orig)(thisObj);
    uiManager = thisObj;
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
    return reinterpret_cast<decltype(wait_resize_ui_hook) * > (wait_resize_ui_orig)(
            thisObj, isPortrait, isShowOrientationGuide);
}

void *set_anti_aliasing_orig = nullptr;

void set_anti_aliasing_hook(int  /*level*/) {
    reinterpret_cast<decltype(set_anti_aliasing_hook) * > (set_anti_aliasing_orig)(g_anti_aliasing);
}

void *set_shadowResolution_orig = nullptr;

void set_shadowResolution_hook(int  /*level*/) {
    reinterpret_cast<decltype(set_shadowResolution_hook) *>(set_shadowResolution_orig)(3);
}

void *set_anisotropicFiltering_orig = nullptr;

void set_anisotropicFiltering_hook(int  /*mode*/) {
    reinterpret_cast<decltype(set_anisotropicFiltering_hook) *>(set_anisotropicFiltering_orig)(2);
}

void *set_shadows_orig = nullptr;

void set_shadows_hook(int  /*quality*/) {
    reinterpret_cast<decltype(set_shadows_hook) *>(set_shadows_orig)(2);
}

void *set_softVegetation_orig = nullptr;

void set_softVegetation_hook(bool  /*enable*/) {
    reinterpret_cast<decltype(set_softVegetation_hook) *>(set_softVegetation_orig)(true);
}

void *set_realtimeReflectionProbes_orig = nullptr;

void set_realtimeReflectionProbes_hook(bool  /*enable*/) {
    reinterpret_cast<decltype(set_realtimeReflectionProbes_hook) *>(set_realtimeReflectionProbes_orig)(
            true);
}

void *Light_set_shadowResolution_orig = nullptr;

void Light_set_shadowResolution_hook(Il2CppObject *thisObj, int  /*level*/) {
    reinterpret_cast<decltype(Light_set_shadowResolution_hook) *>(Light_set_shadowResolution_orig)(
            thisObj, 3);
}

Il2CppObject *(*display_get_main)();

int (*get_system_width)(Il2CppObject *thisObj);

int (*get_system_height)(Il2CppObject *thisObj);

void *set_resolution_orig = nullptr;

void set_resolution_hook(int width, int height, bool fullscreen) {
    const int systemWidth = get_system_width(display_get_main());
    const int systemHeight = get_system_height(display_get_main());
    // Unity 2019 not invert width, height on landscape
    if ((width > height && systemWidth < systemHeight) || g_force_landscape) {
        if (g_ui_use_system_resolution) {
            reinterpret_cast<decltype(set_resolution_hook) * > (set_resolution_orig)(
                    systemHeight, systemWidth,
                    fullscreen);
            return;
        }
    }
    if (g_ui_use_system_resolution) {
        reinterpret_cast<decltype(set_resolution_hook) * > (set_resolution_orig)(
                systemWidth, systemHeight,
                fullscreen);
    } else {
        reinterpret_cast<decltype(set_resolution_hook) * > (set_resolution_orig)(
                width, height,
                fullscreen);
    }
}

void *GraphicSettings_GetVirtualResolution_orig = nullptr;

Vector2Int_t GraphicSettings_GetVirtualResolution_hook(Il2CppObject *thisObj) {
    auto res = reinterpret_cast<decltype(GraphicSettings_GetVirtualResolution_hook) *>(
            GraphicSettings_GetVirtualResolution_orig
    )(thisObj);
    // LOGD("GraphicSettings_GetVirtualResolution %d %d", res.x, res.y);
    return res;
}

void *GraphicSettings_GetVirtualResolution3D_orig = nullptr;

Vector2Int_t
GraphicSettings_GetVirtualResolution3D_hook(Il2CppObject *thisObj, bool isForcedWideAspect) {
    auto resolution = reinterpret_cast<decltype(GraphicSettings_GetVirtualResolution3D_hook) *>(GraphicSettings_GetVirtualResolution3D_orig)(
            thisObj, isForcedWideAspect);
    resolution.x = static_cast<int>(roundf(
            static_cast<float>(resolution.x) * g_resolution_3d_scale));
    resolution.y = static_cast<int>(roundf(
            static_cast<float>(resolution.y) * g_resolution_3d_scale));
    return resolution;
}

void *PathResolver_GetLocalPath_orig = nullptr;

Il2CppString *PathResolver_GetLocalPath_hook(Il2CppObject *thisObj, int kind, Il2CppString *hname) {
    auto hnameU8 = localify::u16_u8(hname->start_char);
    if (g_replace_assets.find(hnameU8) != g_replace_assets.end()) {
        auto &replaceAsset = g_replace_assets.at(hnameU8);
        return il2cpp_string_new(replaceAsset.path.data());
    }
    return reinterpret_cast<decltype(PathResolver_GetLocalPath_hook) *>(PathResolver_GetLocalPath_orig)(
            thisObj, kind, hname);
}

void *apply_graphics_quality_orig = nullptr;

void apply_graphics_quality_hook(Il2CppObject *thisObj, int  /*quality*/, bool  /*force*/) {
    reinterpret_cast<decltype(apply_graphics_quality_hook) * >
    (apply_graphics_quality_orig)(thisObj,
                                  g_graphics_quality,
                                  true);
}

void *assetbundle_LoadFromFile_orig = nullptr;

Il2CppObject *assetbundle_LoadFromFile_hook(Il2CppString *path) {
    stringstream pathStream(localify::u16_u8(path->start_char));
    string segment;
    vector<string> split;
    while (getline(pathStream, segment, '/')) {
        split.push_back(segment);
    }
    if (g_replace_assets.find(split[split.size() - 1]) != g_replace_assets.end()) {
        auto &replaceAsset = g_replace_assets.at(split[split.size() - 1]);
        replaceAsset.asset = reinterpret_cast<decltype(assetbundle_LoadFromFile_hook) *>(assetbundle_LoadFromFile_orig)(
                il2cpp_string_new(replaceAsset.path.data()));
        return replaceAsset.asset;
    }
    return reinterpret_cast<decltype(assetbundle_LoadFromFile_hook) *>(assetbundle_LoadFromFile_orig)(
            path);
}

void *assetbundle_load_asset_orig = nullptr;

Il2CppObject *
assetbundle_load_asset_hook(Il2CppObject *thisObj, Il2CppString *name, const Il2CppType *type) {
    stringstream pathStream(localify::u16_u8(name->start_char));
    string segment;
    vector<string> split;
    while (getline(pathStream, segment, '/')) {
        split.emplace_back(segment);
    }
    auto &fileName = split.back();
    if (find_if(replaceAssetNames.begin(), replaceAssetNames.end(), [fileName](const string &item) {
        return item.find(fileName) != string::npos;
    }) != replaceAssetNames.end()) {
        return reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                replaceAssets, il2cpp_string_new(fileName.data()), type);
    }
    auto *asset = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
            thisObj, name, type);
    if (asset->klass->name == "GameObject"s) {
        auto getComponent = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                               Il2CppType *)>(il2cpp_class_get_method_from_name(
                asset->klass, "GetComponent", 1)->methodPointer);
        auto *assetHolder = getComponent(asset,
                                         reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                                 "umamusume.dll", "Gallop",
                                                 "AssetHolder")));
        if (assetHolder != nullptr) {
            auto *objectList = reinterpret_cast<Il2CppObject *(*)(
                    Il2CppObject *)>(il2cpp_class_get_method_from_name(assetHolder->klass,
                                                                       "get_ObjectList",
                                                                       0)->methodPointer)(
                    assetHolder);
            FieldInfo *itemsField = il2cpp_class_get_field_from_name(objectList->klass, "_items");
            Il2CppArray *arr;
            il2cpp_field_get_value(objectList, itemsField, &arr);
            for (int i = 0; i < arr->max_length; i++) {
                auto *pair = reinterpret_cast<Il2CppObject *>(arr->vector[i]);
                auto *field = il2cpp_class_get_field_from_name(pair->klass, "Value");
                Il2CppObject *obj;
                il2cpp_field_get_value(pair, field, &obj);
                if (obj != nullptr) {
                    if (obj->klass->name == "Texture2D"s) {
                        auto *uobject_name = uobject_get_name(obj);
                        if (!localify::u16_u8(uobject_name->start_char).empty()) {
                            auto *newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                                    replaceAssets,
                                    uobject_name,
                                    reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                            "UnityEngine.CoreModule.dll",
                                            "UnityEngine", "Texture2D")));
                            if (newTexture != nullptr) {
                                reinterpret_cast<void (*)(Il2CppObject *, int)>(
                                        il2cpp_symbols::get_method_pointer(
                                                "UnityEngine.CoreModule.dll", "UnityEngine",
                                                "Object", "set_hideFlags", 1)
                                )(newTexture, 32);
                                il2cpp_field_set_value(pair, field, newTexture);
                            }
                        }
                    }
                    if (obj->klass->name == "Material"s) {
                        auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                                Il2CppObject *)>(il2cpp_class_get_method_from_name(obj->klass,
                                                                                   "get_mainTexture",
                                                                                   0)->methodPointer);
                        auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                                  Il2CppObject *)>(il2cpp_class_get_method_from_name(
                                obj->klass, "set_mainTexture", 1)->methodPointer);
                        auto *mainTexture = get_mainTexture(obj);
                        if (mainTexture != nullptr) {
                            auto *uobject_name = uobject_get_name(mainTexture);
                            if (!localify::u16_u8(uobject_name->start_char).empty()) {
                                auto *newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                                        replaceAssets,
                                        uobject_name,
                                        reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                                "UnityEngine.CoreModule.dll",
                                                "UnityEngine", "Texture2D")));
                                if (newTexture != nullptr) {
                                    reinterpret_cast<void (*)(Il2CppObject *, int)>(
                                            il2cpp_symbols::get_method_pointer(
                                                    "UnityEngine.CoreModule.dll", "UnityEngine",
                                                    "Object", "set_hideFlags", 1)
                                    )(newTexture, 32);
                                    set_mainTexture(obj, newTexture);
                                }
                            }
                        }

                    }
                }
            }
        }
    }
    return asset;
}

void *assetbundle_unload_orig = nullptr;

void assetbundle_unload_hook(Il2CppObject *thisObj, bool unloadAllLoadedObjects) {
    for (auto &pair: g_replace_assets) {
        if (pair.second.asset == thisObj) {
            pair.second.asset = nullptr;
        }
    }
    reinterpret_cast<decltype(assetbundle_unload_hook) * > (assetbundle_unload_orig)(thisObj,
                                                                                     unloadAllLoadedObjects);
}

void *AssetBundleRequest_GetResult_orig = nullptr;

Il2CppObject *AssetBundleRequest_GetResult_hook(Il2CppObject *thisObj) {
    auto *obj = reinterpret_cast<decltype(AssetBundleRequest_GetResult_hook) *>(AssetBundleRequest_GetResult_orig)(
            thisObj);
    if (obj != nullptr) {
        auto *name = uobject_get_name(obj);
        auto u8Name = localify::u16_u8(name->start_char);
        if (find(replaceAssetNames.begin(), replaceAssetNames.end(), u8Name) !=
            replaceAssetNames.end()) {
            return reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                    replaceAssets, name, il2cpp_class_get_type(obj->klass));
        }
    }
    return obj;
}

void *resources_load_orig = nullptr;

Il2CppObject *resources_load_hook(Il2CppString *path, Il2CppType *type) {
    string const u8Name = localify::u16_u8(path->start_char);
    if (u8Name == "ui/views/titleview"s) {
        if (find(replaceAssetNames.begin(), replaceAssetNames.end(),
                 "assets/title/utx_obj_title_logo_umamusume.png") != replaceAssetNames.end()) {
            auto *gameObj = reinterpret_cast<decltype(resources_load_hook) *>(resources_load_orig)(
                    path, type);
            auto getComponent = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                   Il2CppType *)>(il2cpp_class_get_method_from_name(
                    gameObj->klass, "GetComponent", 1)->methodPointer);
            auto *component = getComponent(gameObj,
                                           reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                                   "umamusume.dll", "Gallop",
                                                   "TitleView")));

            auto *imgField = il2cpp_class_get_field_from_name(component->klass, "TitleLogoImage");
            Il2CppObject *imgCommon;
            il2cpp_field_get_value(component, imgField, &imgCommon);
            auto *texture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                    replaceAssets,
                    il2cpp_string_new("assets/title/utx_obj_title_logo_umamusume.png"),
                    reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                                  "UnityEngine",
                                                                  "Texture2D")));
            auto *m_TextureField = il2cpp_class_get_field_from_name(imgCommon->klass->parent,
                                                                    "m_Texture");
            il2cpp_field_set_value(imgCommon, m_TextureField, texture);
            return gameObj;
        }
    }
    return reinterpret_cast<decltype(resources_load_hook) *>(resources_load_orig)(path, type);

}

void *Sprite_get_texture_orig = nullptr;

Il2CppObject *Sprite_get_texture_hook(Il2CppObject *thisObj) {
    auto texture2D = reinterpret_cast<decltype(Sprite_get_texture_hook) *>(Sprite_get_texture_orig)(
            thisObj);
    auto uobject_name = uobject_get_name(texture2D);
    if (!localify::u16_u8(uobject_name->start_char).empty()) {
        auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                replaceAssets,
                uobject_name,
                reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                              "UnityEngine",
                                                              "Texture2D")));
        if (newTexture) {
            reinterpret_cast<void (*)(Il2CppObject *, int)>(
                    il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine",
                                                       "Object", "set_hideFlags", 1)
            )(newTexture, 32);
            return newTexture;
        }
    }
    return texture2D;
}

void *Renderer_get_material_orig = nullptr;

Il2CppObject *Renderer_get_material_hook(Il2CppObject *thisObj) {
    auto material = reinterpret_cast<decltype(Renderer_get_material_hook) *>(Renderer_get_material_orig)(
            thisObj);
    if (material) {
        auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_class_get_method_from_name(material->klass,
                                                                   "get_mainTexture",
                                                                   0)->methodPointer);
        auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                  Il2CppObject *)>(il2cpp_class_get_method_from_name(
                material->klass, "set_mainTexture", 1)->methodPointer);
        auto mainTexture = get_mainTexture(material);
        if (mainTexture) {
            auto uobject_name = uobject_get_name(mainTexture);
            if (!localify::u16_u8(uobject_name->start_char).empty()) {
                auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                        replaceAssets,
                        uobject_name,
                        reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                                      "UnityEngine",
                                                                      "Texture2D")));
                if (newTexture) {
                    reinterpret_cast<void (*)(Il2CppObject *, int)>(
                            il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                               "UnityEngine", "Object",
                                                               "set_hideFlags", 1)
                    )(newTexture, 32);
                    set_mainTexture(material, newTexture);
                }
            }
        }
    }
    return material;
}

void *Renderer_get_materials_orig = nullptr;

Il2CppArray *Renderer_get_materials_hook(Il2CppObject *thisObj) {
    auto materials = reinterpret_cast<decltype(Renderer_get_materials_hook) *>(Renderer_get_materials_orig)(
            thisObj);
    for (int i = 0; i < materials->max_length; i++) {
        auto material = reinterpret_cast<Il2CppObject *>(materials->vector[i]);
        if (material) {
            auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                    Il2CppObject *)>(il2cpp_class_get_method_from_name(material->klass,
                                                                       "get_mainTexture",
                                                                       0)->methodPointer);
            auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                      Il2CppObject *)>(il2cpp_class_get_method_from_name(
                    material->klass, "set_mainTexture", 1)->methodPointer);
            auto mainTexture = get_mainTexture(material);
            if (mainTexture) {
                auto uobject_name = uobject_get_name(mainTexture);
                if (!localify::u16_u8(uobject_name->start_char).empty()) {
                    auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                            replaceAssets,
                            uobject_name,
                            reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                    "UnityEngine.CoreModule.dll",
                                    "UnityEngine", "Texture2D")));
                    if (newTexture) {
                        reinterpret_cast<void (*)(Il2CppObject *, int)>(
                                il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                   "UnityEngine", "Object",
                                                                   "set_hideFlags", 1)
                        )(newTexture, 32);
                        set_mainTexture(material, newTexture);
                    }
                }
            }
        }
    }
    return materials;
}

void *Renderer_get_sharedMaterial_orig = nullptr;

Il2CppObject *Renderer_get_sharedMaterial_hook(Il2CppObject *thisObj) {
    auto material = reinterpret_cast<decltype(Renderer_get_sharedMaterial_hook) *>(Renderer_get_sharedMaterial_orig)(
            thisObj);
    if (material) {
        auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_class_get_method_from_name(material->klass,
                                                                   "get_mainTexture",
                                                                   0)->methodPointer);
        auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                  Il2CppObject *)>(il2cpp_class_get_method_from_name(
                material->klass, "set_mainTexture", 1)->methodPointer);
        auto mainTexture = get_mainTexture(material);
        if (mainTexture) {
            auto uobject_name = uobject_get_name(mainTexture);
            if (!localify::u16_u8(uobject_name->start_char).empty()) {
                auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                        replaceAssets,
                        uobject_name,
                        reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                                      "UnityEngine",
                                                                      "Texture2D")));
                if (newTexture) {
                    reinterpret_cast<void (*)(Il2CppObject *, int)>(
                            il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                               "UnityEngine", "Object",
                                                               "set_hideFlags", 1)
                    )(newTexture, 32);
                    set_mainTexture(material, newTexture);
                }
            }
        }
    }
    return material;
}

void *Renderer_get_sharedMaterials_orig = nullptr;

Il2CppArray *Renderer_get_sharedMaterials_hook(Il2CppObject *thisObj) {
    auto materials = reinterpret_cast<decltype(Renderer_get_sharedMaterials_hook) *>(Renderer_get_sharedMaterials_orig)(
            thisObj);
    for (int i = 0; i < materials->max_length; i++) {
        auto material = reinterpret_cast<Il2CppObject *>(materials->vector[i]);
        if (material) {
            auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                    Il2CppObject *)>(il2cpp_class_get_method_from_name(material->klass,
                                                                       "get_mainTexture",
                                                                       0)->methodPointer);
            auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                      Il2CppObject *)>(il2cpp_class_get_method_from_name(
                    material->klass, "set_mainTexture", 1)->methodPointer);
            auto mainTexture = get_mainTexture(material);
            if (mainTexture) {
                auto uobject_name = uobject_get_name(mainTexture);
                if (!localify::u16_u8(uobject_name->start_char).empty()) {
                    auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                            replaceAssets,
                            uobject_name,
                            reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                    "UnityEngine.CoreModule.dll",
                                    "UnityEngine", "Texture2D")));
                    if (newTexture) {
                        reinterpret_cast<void (*)(Il2CppObject *, int)>(
                                il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                   "UnityEngine", "Object",
                                                                   "set_hideFlags", 1)
                        )(newTexture, 32);
                        set_mainTexture(material, newTexture);
                    }
                }
            }
        }
    }
    return materials;
}

void *Renderer_set_material_orig = nullptr;

void Renderer_set_material_hook(Il2CppObject *thisObj, Il2CppObject *material) {
    if (material) {
        auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_class_get_method_from_name(material->klass,
                                                                   "get_mainTexture",
                                                                   0)->methodPointer);
        auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                  Il2CppObject *)>(il2cpp_class_get_method_from_name(
                material->klass, "set_mainTexture", 1)->methodPointer);
        auto mainTexture = get_mainTexture(material);
        if (mainTexture) {
            auto uobject_name = uobject_get_name(mainTexture);
            if (!localify::u16_u8(uobject_name->start_char).empty()) {
                auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                        replaceAssets,
                        uobject_name,
                        reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                                      "UnityEngine",
                                                                      "Texture2D")));
                if (newTexture) {
                    reinterpret_cast<void (*)(Il2CppObject *, int)>(
                            il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                               "UnityEngine", "Object",
                                                               "set_hideFlags", 1)
                    )(newTexture, 32);
                    set_mainTexture(material, newTexture);
                }
            }
        }
    }
    reinterpret_cast<decltype(Renderer_set_material_hook) *>(Renderer_set_material_orig)(thisObj,
                                                                                         material);
}

void *Renderer_set_materials_orig = nullptr;

void Renderer_set_materials_hook(Il2CppObject *thisObj, Il2CppArray *materials) {
    for (int i = 0; i < materials->max_length; i++) {
        auto material = reinterpret_cast<Il2CppObject *>(materials->vector[i]);
        if (material) {
            auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                    Il2CppObject *)>(il2cpp_class_get_method_from_name(material->klass,
                                                                       "get_mainTexture",
                                                                       0)->methodPointer);
            auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                      Il2CppObject *)>(il2cpp_class_get_method_from_name(
                    material->klass, "set_mainTexture", 1)->methodPointer);
            auto mainTexture = get_mainTexture(material);
            if (mainTexture) {
                auto uobject_name = uobject_get_name(mainTexture);
                if (!localify::u16_u8(uobject_name->start_char).empty()) {
                    auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                            replaceAssets,
                            uobject_name,
                            reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                    "UnityEngine.CoreModule.dll",
                                    "UnityEngine", "Texture2D")));
                    if (newTexture) {
                        reinterpret_cast<void (*)(Il2CppObject *, int)>(
                                il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                   "UnityEngine", "Object",
                                                                   "set_hideFlags", 1)
                        )(newTexture, 32);
                        set_mainTexture(material, newTexture);
                    }
                }
            }
        }
    }
    reinterpret_cast<decltype(Renderer_set_materials_hook) *>(Renderer_set_materials_orig)(thisObj,
                                                                                           materials);
}

void *Renderer_set_sharedMaterial_orig = nullptr;

void Renderer_set_sharedMaterial_hook(Il2CppObject *thisObj, Il2CppObject *material) {
    if (material) {
        auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_class_get_method_from_name(material->klass,
                                                                   "get_mainTexture",
                                                                   0)->methodPointer);
        auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                  Il2CppObject *)>(il2cpp_class_get_method_from_name(
                material->klass, "set_mainTexture", 1)->methodPointer);
        auto mainTexture = get_mainTexture(material);
        if (mainTexture) {
            auto uobject_name = uobject_get_name(mainTexture);
            if (!localify::u16_u8(uobject_name->start_char).empty()) {
                auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                        replaceAssets,
                        uobject_name,
                        reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                                      "UnityEngine",
                                                                      "Texture2D")));
                if (newTexture) {
                    reinterpret_cast<void (*)(Il2CppObject *, int)>(
                            il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                               "UnityEngine", "Object",
                                                               "set_hideFlags", 1)
                    )(newTexture, 32);
                    set_mainTexture(material, newTexture);
                }
            }
        }
    }
    reinterpret_cast<decltype(Renderer_set_sharedMaterial_hook) *>(Renderer_set_sharedMaterial_orig)(
            thisObj, material);
}

void *Renderer_set_sharedMaterials_orig = nullptr;

void Renderer_set_sharedMaterials_hook(Il2CppObject *thisObj, Il2CppArray *materials) {
    for (int i = 0; i < materials->max_length; i++) {
        auto material = reinterpret_cast<Il2CppObject *>(materials->vector[i]);
        if (material) {
            auto get_mainTexture = reinterpret_cast<Il2CppObject *(*)(
                    Il2CppObject *)>(il2cpp_class_get_method_from_name(material->klass,
                                                                       "get_mainTexture",
                                                                       0)->methodPointer);
            auto set_mainTexture = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                      Il2CppObject *)>(il2cpp_class_get_method_from_name(
                    material->klass, "set_mainTexture", 1)->methodPointer);
            auto mainTexture = get_mainTexture(material);
            if (mainTexture) {
                auto uobject_name = uobject_get_name(mainTexture);
                if (!localify::u16_u8(uobject_name->start_char).empty()) {
                    auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                            replaceAssets,
                            uobject_name,
                            reinterpret_cast<Il2CppType *>(GetRuntimeType(
                                    "UnityEngine.CoreModule.dll",
                                    "UnityEngine", "Texture2D")));
                    if (newTexture) {
                        reinterpret_cast<void (*)(Il2CppObject *, int)>(
                                il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                   "UnityEngine", "Object",
                                                                   "set_hideFlags", 1)
                        )(newTexture, 32);
                        set_mainTexture(material, newTexture);
                    }
                }
            }
        }
    }
    reinterpret_cast<decltype(Renderer_set_sharedMaterials_hook) *>(Renderer_set_sharedMaterials_orig)(
            thisObj, materials);
}

void *Material_set_mainTexture_orig = nullptr;

void Material_set_mainTexture_hook(Il2CppObject *thisObj, Il2CppObject *texture) {
    if (texture) {
        if (!localify::u16_u8(uobject_get_name(texture)->start_char).empty()) {
            auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                    replaceAssets,
                    uobject_get_name(texture),
                    reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                                  "UnityEngine",
                                                                  "Texture2D")));
            if (newTexture) {
                reinterpret_cast<void (*)(Il2CppObject *, int)>(
                        il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                           "UnityEngine", "Object", "set_hideFlags",
                                                           1)
                )(newTexture, 32);
                reinterpret_cast<decltype(Material_set_mainTexture_hook) *>(Material_set_mainTexture_orig)(
                        thisObj, newTexture);
                return;
            }
        }
    }
    reinterpret_cast<decltype(Material_set_mainTexture_hook) *>(Material_set_mainTexture_orig)(
            thisObj, texture);
}

void *Material_get_mainTexture_orig = nullptr;

Il2CppObject *Material_get_mainTexture_hook(Il2CppObject *thisObj) {
    auto texture = reinterpret_cast<decltype(Material_get_mainTexture_hook) *>(Material_get_mainTexture_orig)(
            thisObj);
    if (texture) {
        auto uobject_name = uobject_get_name(texture);
        if (!localify::u16_u8(uobject_name->start_char).empty()) {
            auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                    replaceAssets,
                    uobject_name,
                    reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                                  "UnityEngine",
                                                                  "Texture2D")));
            if (newTexture) {
                reinterpret_cast<void (*)(Il2CppObject *, int)>(
                        il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                           "UnityEngine", "Object", "set_hideFlags",
                                                           1)
                )(newTexture, 32);
                return newTexture;
            }
        }
    }
    return texture;
}

void *Material_SetTextureI4_orig = nullptr;

void Material_SetTextureI4_hook(Il2CppObject *thisObj, int nameID, Il2CppObject *texture) {
    if (texture && !localify::u16_u8(uobject_get_name(texture)->start_char).empty()) {
        auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                replaceAssets,
                uobject_get_name(texture),
                reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                              "UnityEngine",
                                                              "Texture2D")));
        if (newTexture) {
            reinterpret_cast<void (*)(Il2CppObject *, int)>(
                    il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine",
                                                       "Object", "set_hideFlags", 1)
            )(newTexture, 32);
            reinterpret_cast<decltype(Material_SetTextureI4_hook) *>(Material_SetTextureI4_orig)(
                    thisObj, nameID, newTexture);
            return;
        }
    }
    reinterpret_cast<decltype(Material_SetTextureI4_hook) *>(Material_SetTextureI4_orig)(thisObj,
                                                                                         nameID,
                                                                                         texture);
}

void *CharaPropRendererAccessor_SetTexture_orig = nullptr;

void CharaPropRendererAccessor_SetTexture_hook(Il2CppObject *thisObj, Il2CppObject *texture) {
    if (!localify::u16_u8(uobject_get_name(texture)->start_char).empty()) {
        auto newTexture = reinterpret_cast<decltype(assetbundle_load_asset_hook) *>(assetbundle_load_asset_orig)(
                replaceAssets,
                uobject_get_name(texture),
                reinterpret_cast<Il2CppType *>(GetRuntimeType("UnityEngine.CoreModule.dll",
                                                              "UnityEngine",
                                                              "Texture2D")));
        if (newTexture) {
            reinterpret_cast<void (*)(Il2CppObject *, int)>(
                    il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll", "UnityEngine",
                                                       "Object", "set_hideFlags", 1)
            )(newTexture, 32);
            reinterpret_cast<decltype(CharaPropRendererAccessor_SetTexture_hook) *>(CharaPropRendererAccessor_SetTexture_orig)(
                    thisObj, newTexture);
            return;
        }
    }
    reinterpret_cast<decltype(CharaPropRendererAccessor_SetTexture_hook) *>(CharaPropRendererAccessor_SetTexture_orig)(
            thisObj, texture);
}

void *ChangeScreenOrientation_orig = nullptr;

Il2CppObject *ChangeScreenOrientation_hook(ScreenOrientation targetOrientation, bool isForce) {
    return reinterpret_cast<decltype(ChangeScreenOrientation_hook) * >
    (ChangeScreenOrientation_orig)(
            g_force_landscape ? ScreenOrientation::Landscape
                              : targetOrientation, isForce);
}

void *ChangeScreenOrientationPortraitAsync_orig = nullptr;

Il2CppObject *ChangeScreenOrientationPortraitAsync_hook() {
    return reinterpret_cast<Il2CppObject *(*)()>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop",
            "Screen", "ChangeScreenOrientationLandscapeAsync", -1))();
}

void *CanvasScaler_set_referenceResolution_orig = nullptr;

void CanvasScaler_set_referenceResolution_hook(Il2CppObject *thisObj, Vector2_t res) {
    if (g_force_landscape) {
        res.x /= (max(1.0f, res.x / 1920.f) * g_force_landscape_ui_scale);
        res.y /= (max(1.0f, res.y / 1080.f) * g_force_landscape_ui_scale);
    }
    return reinterpret_cast<decltype(CanvasScaler_set_referenceResolution_hook) * >
    (CanvasScaler_set_referenceResolution_orig)(
            thisObj, res);
}

void *SetResolution_orig = nullptr;

void SetResolution_hook(int w, int h, bool fullscreen, bool forceUpdate) {
    if (!resolutionIsSet || GetUnityVersion() == Unity2020) {
        if (sceneManager || GetUnityVersion() == Unity2019 && w < h) {
            resolutionIsSet = true;
        }
        reinterpret_cast<decltype(SetResolution_hook) * > (SetResolution_orig)(w, h, fullscreen,
                                                                               forceUpdate);
        if (g_force_landscape) {
            if (GetUnityVersion() == Unity2019 || (w < h && GetUnityVersion() == Unity2020)) {
                reinterpret_cast<decltype(set_resolution_hook) * > (set_resolution_orig)(h, w,
                                                                                         fullscreen);
            }
        }
    }
}

int (*Screen_get_width)();

int (*Screen_get_height)();

void *Screen_set_orientation_orig = nullptr;

void Screen_set_orientation_hook(ScreenOrientation orientation) {
    if ((orientation == ScreenOrientation::Portrait ||
         orientation == ScreenOrientation::PortraitUpsideDown) && g_force_landscape) {
        orientation = ScreenOrientation::Landscape;
    }
    reinterpret_cast<decltype(Screen_set_orientation_hook) * > (Screen_set_orientation_orig)(
            orientation);
}

void *DeviceOrientationGuide_Show_orig = nullptr;

void DeviceOrientationGuide_Show_hook(Il2CppObject *thisObj, bool isTargetOrientationPortrait,
                                      int target) {
    reinterpret_cast<decltype(DeviceOrientationGuide_Show_hook) * >
    (DeviceOrientationGuide_Show_orig)(
            thisObj,
            !g_force_landscape && isTargetOrientationPortrait,
            g_force_landscape ? 2 : target);
}

void *NowLoading_Show_orig = nullptr;

void NowLoading_Show_hook(Il2CppObject *thisObj, int type, Il2CppDelegate *onComplete,
                          float overrideDuration) {
    // NowLoadingOrientation
    if (type == 2 && (g_force_landscape || !g_ui_loading_show_orientation_guide)) {
        // NowLoadingTips
        type = 0;
    }
    if (!g_hide_now_loading) {
        reinterpret_cast<decltype(NowLoading_Show_hook) *>(NowLoading_Show_orig)(
                thisObj,
                type,
                onComplete, overrideDuration);
    }
    if (onComplete) {
        if (g_hide_now_loading) {
            reinterpret_cast<void (*)(Il2CppObject *)>(onComplete->method_ptr)(onComplete->target);
        } else {
            const bool isShown = reinterpret_cast<bool (*)(
                    Il2CppObject *)>(il2cpp_class_get_method_from_name(
                    thisObj->klass, "IsShown", 0)->methodPointer)(thisObj);
            if (!isShown) {
                LOGI("NowLoading::Show: onComplete not called, calling onComplete manually...");
                reinterpret_cast<void (*)(Il2CppObject *)>(onComplete->method_ptr)(
                        onComplete->target);
            }
        }
    }
}

void *NowLoading_Show2_orig = nullptr;

void NowLoading_Show2_hook(Il2CppObject *thisObj, int type, Il2CppDelegate *onComplete,
                           Il2CppObject *overrideDuration, int easeType) {
    // NowLoadingOrientation
    if (type == 2 && (g_force_landscape || !g_ui_loading_show_orientation_guide)) {
        // NowLoadingTips
        type = 0;
    }
    if (!g_hide_now_loading) {
        reinterpret_cast<decltype(NowLoading_Show2_hook) *>(NowLoading_Show2_orig)(
                thisObj,
                type,
                onComplete, overrideDuration, easeType);
    }
    if (onComplete) {
        if (g_hide_now_loading) {
            reinterpret_cast<void (*)(Il2CppObject *)>(onComplete->method_ptr)(onComplete->target);
        } else {
            const bool isShown = reinterpret_cast<bool (*)(
                    Il2CppObject *)>(il2cpp_class_get_method_from_name(
                    thisObj->klass, "IsShown", 0)->methodPointer)(thisObj);
            if (!isShown) {
                LOGI("NowLoading::Show: onComplete not called, calling onComplete manually...");
                reinterpret_cast<void (*)(Il2CppObject *)>(onComplete->method_ptr)(
                        onComplete->target);
            }
        }
    }
}

void *NowLoading_Hide_orig = nullptr;

void NowLoading_Hide_hook(Il2CppObject *thisObj, Il2CppDelegate *onComplete) {
    if (onComplete) {
        reinterpret_cast<void (*)(Il2CppObject *)>(onComplete->method_ptr)(onComplete->target);
    }
}

void *NowLoading_Hide2_orig = nullptr;

void NowLoading_Hide2_hook(Il2CppObject *thisObj, Il2CppDelegate *onComplete,
                           Il2CppObject *overrideDuration, int easeType) {
    if (onComplete) {
        reinterpret_cast<void (*)(Il2CppObject *)>(onComplete->method_ptr)(onComplete->target);
    }
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

void *SafetyNet_OnSuccess_orig = nullptr;

void SafetyNet_OnSuccess_hook(Il2CppObject *thisObj, Il2CppString *jws) {
    reinterpret_cast<decltype(SafetyNet_OnSuccess_hook) *>(SafetyNet_OnSuccess_orig)(thisObj, jws);
}

void *SafetyNet_OnError_orig = nullptr;

void SafetyNet_OnError_hook(Il2CppObject *thisObj, Il2CppString *error) {
    reinterpret_cast<decltype(SafetyNet_OnSuccess_hook) *>(SafetyNet_OnSuccess_orig)(thisObj,
                                                                                     error);
}

void *SafetyNet_GetSafetyNetStatus_orig = nullptr;

void SafetyNet_GetSafetyNetStatus_hook(Il2CppString *apiKey, Il2CppString *nonce,
                                       Il2CppDelegate *onSuccess, Il2CppDelegate *onError) {
    reinterpret_cast<decltype(SafetyNet_GetSafetyNetStatus_hook) *>(SafetyNet_GetSafetyNetStatus_orig)(
            apiKey,
            nonce, onSuccess, onSuccess);
}

void *Device_IsIllegalUser_orig = nullptr;

Boolean Device_IsIllegalUser_hook() {
    return Boolean{.m_value = false};
}

struct MoviePlayerHandle {
};

Il2CppObject *(*MoviePlayerBase_get_MovieInfo)(Il2CppObject *thisObj);

Il2CppObject *(*MovieManager_GetMovieInfo)(Il2CppObject *thisObj, MoviePlayerHandle playerHandle);

void *MovieManager_SetImageUvRect_orig = nullptr;

void
MovieManager_SetImageUvRect_hook(Il2CppObject *thisObj, MoviePlayerHandle playerHandle, Rect_t uv) {
    auto movieInfo = MovieManager_GetMovieInfo(thisObj, playerHandle);
    auto widthField = il2cpp_class_get_field_from_name(movieInfo->klass, "width");
    auto heightField = il2cpp_class_get_field_from_name(movieInfo->klass, "height");
    unsigned int movieWidth, movieHeight;
    il2cpp_field_get_value(movieInfo, widthField, &movieWidth);
    il2cpp_field_get_value(movieInfo, heightField, &movieHeight);
    if (movieWidth < movieHeight) {
        auto width = static_cast<float>(Screen_get_width());
        auto height = static_cast<float>(Screen_get_height());
        if (roundf(1080 / (max(1.0f, height / 1080.f) * g_force_landscape_ui_scale)) ==
            static_cast<float>(uv.y)) {
            uv.y = static_cast<short>(width);
        }
        uv.x = static_cast<short>(height);
    }

    reinterpret_cast<decltype(MovieManager_SetImageUvRect_hook) *>(MovieManager_SetImageUvRect_orig)(
            thisObj, playerHandle, uv);
}

void *MovieManager_SetScreenSize_orig = nullptr;

void MovieManager_SetScreenSize_hook(Il2CppObject *thisObj, MoviePlayerHandle playerHandle,
                                     Vector2_t screenSize) {
    auto movieInfo = MovieManager_GetMovieInfo(thisObj, playerHandle);
    auto widthField = il2cpp_class_get_field_from_name(movieInfo->klass, "width");
    auto heightField = il2cpp_class_get_field_from_name(movieInfo->klass, "height");
    unsigned int movieWidth, movieHeight;
    il2cpp_field_get_value(movieInfo, widthField, &movieWidth);
    il2cpp_field_get_value(movieInfo, heightField, &movieHeight);
    if (movieWidth < movieHeight) {
        auto width = static_cast<float>(Screen_get_width());
        auto height = static_cast<float>(Screen_get_height());
        if (roundf(1080 / (max(1.0f, height / 1080.f) * g_force_landscape_ui_scale)) ==
            screenSize.y) {
            screenSize.y = width;
        }
        screenSize.x = height;
    }

    reinterpret_cast<decltype(MovieManager_SetScreenSize_hook) *>(MovieManager_SetScreenSize_orig)(
            thisObj, playerHandle, screenSize);
}

void *MoviePlayerForUI_AdjustScreenSize_orig = nullptr;

void MoviePlayerForUI_AdjustScreenSize_hook(Il2CppObject *thisObj, Vector2_t dispRectWH,
                                            bool isPanScan) {
    auto movieInfo = MoviePlayerBase_get_MovieInfo(thisObj);
    auto widthField = il2cpp_class_get_field_from_name(movieInfo->klass, "width");
    auto heightField = il2cpp_class_get_field_from_name(movieInfo->klass, "height");
    unsigned int movieWidth, movieHeight;
    il2cpp_field_get_value(movieInfo, widthField, &movieWidth);
    il2cpp_field_get_value(movieInfo, heightField, &movieHeight);
    if (movieWidth < movieHeight) {
        auto width = static_cast<float>(Screen_get_width());
        auto height = static_cast<float>(Screen_get_height());
        if (roundf(1080 / (max(1.0f, height / 1080.f) * g_force_landscape_ui_scale)) ==
            dispRectWH.y) {
            dispRectWH.y = width;
        }
        dispRectWH.x = height;
    }
    reinterpret_cast<decltype(MoviePlayerForUI_AdjustScreenSize_hook) *>(MoviePlayerForUI_AdjustScreenSize_orig)(
            thisObj, dispRectWH, isPanScan);
}

void *MoviePlayerForObj_AdjustScreenSize_orig = nullptr;

void MoviePlayerForObj_AdjustScreenSize_hook(Il2CppObject *thisObj, Vector2_t dispRectWH,
                                             bool isPanScan) {
    auto movieInfo = MoviePlayerBase_get_MovieInfo(thisObj);
    auto widthField = il2cpp_class_get_field_from_name(movieInfo->klass, "width");
    auto heightField = il2cpp_class_get_field_from_name(movieInfo->klass, "height");
    unsigned int movieWidth, movieHeight;
    il2cpp_field_get_value(movieInfo, widthField, &movieWidth);
    il2cpp_field_get_value(movieInfo, heightField, &movieHeight);
    if (movieWidth < movieHeight) {
        auto width = static_cast<float>(Screen_get_width());
        auto height = static_cast<float>(Screen_get_height());
        if (roundf(1080 / (max(1.0f, height / 1080.f) * g_force_landscape_ui_scale)) ==
            dispRectWH.y) {
            dispRectWH.y = width;
        }
        dispRectWH.x = height;
    }
    reinterpret_cast<decltype(MoviePlayerForObj_AdjustScreenSize_hook) *>(MoviePlayerForObj_AdjustScreenSize_orig)(
            thisObj, dispRectWH, isPanScan);
}

void *FrameRateController_OverrideByNormalFrameRate_orig = nullptr;

void FrameRateController_OverrideByNormalFrameRate_hook(Il2CppObject *thisObj, int  /*layer*/) {
    // FrameRateOverrideLayer.SystemValue
    reinterpret_cast<decltype(FrameRateController_OverrideByNormalFrameRate_hook) *>(FrameRateController_OverrideByNormalFrameRate_orig)(
            thisObj, 0);
}

void *FrameRateController_OverrideByMaxFrameRate_orig = nullptr;

void FrameRateController_OverrideByMaxFrameRate_hook(Il2CppObject *thisObj, int  /*layer*/) {
    // FrameRateOverrideLayer.SystemValue
    reinterpret_cast<decltype(FrameRateController_OverrideByMaxFrameRate_hook) *>(FrameRateController_OverrideByMaxFrameRate_orig)(
            thisObj, 0);
}

void *FrameRateController_ResetOverride_orig = nullptr;

void FrameRateController_ResetOverride_hook(Il2CppObject *thisObj, int  /*layer*/) {
    // FrameRateOverrideLayer.SystemValue
    reinterpret_cast<decltype(FrameRateController_ResetOverride_hook) *>(FrameRateController_ResetOverride_orig)(
            thisObj, 0);
}

void *FrameRateController_ReflectionFrameRate_orig = nullptr;

void FrameRateController_ReflectionFrameRate_hook(Il2CppObject * /*thisObj*/) {
    set_fps_hook(30);
}

Il2CppObject *errorDialog = nullptr;

void *DialogCommon_Close_orig = nullptr;

void DialogCommon_Close_hook(Il2CppObject *thisObj) {
    if (thisObj == errorDialog) {
        if (sceneManager) {
            // Home 100
            reinterpret_cast<void (*)(Il2CppObject *, int, Il2CppObject *, Il2CppObject *,
                                      Il2CppObject *, bool)>(
                    il2cpp_class_get_method_from_name(sceneManager->klass, "ChangeView",
                                                      5)->methodPointer
            )(sceneManager, 100, nullptr, nullptr, nullptr, true);
        }
    }
    reinterpret_cast<decltype(DialogCommon_Close_hook) *>(DialogCommon_Close_orig)(thisObj);
}

void *GallopUtil_GotoTitleOnError_orig = nullptr;

void GallopUtil_GotoTitleOnError_hook(Il2CppString * /*text*/) {
    // Bypass SoftwareReset
    auto okText = GetTextIdByName("Common0009");
    auto errorText = GetTextIdByName("Common0071");

    auto dialogData = il2cpp_object_new(
            il2cpp_symbols::get_class("umamusume.dll", "Gallop", "DialogCommon/Data"));
    il2cpp_runtime_object_init(dialogData);
    auto message = GotoTitleError;
    if (Game::currentGameRegion == Game::Region::JAP) {
        message = GotoTitleErrorJa;
    }
    if (Game::currentGameRegion == Game::Region::TWN) {
        message = GotoTitleErrorHan;
    }
    dialogData = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *thisObj,
                                                    unsigned long headerTextId,
                                                    Il2CppString *message,
                                                    Il2CppDelegate *onClickCenterButton,
                                                    unsigned long closeTextId)>(
            il2cpp_class_get_method_from_name(dialogData->klass, "SetSimpleOneButtonMessage",
                                              4)->methodPointer
    )(dialogData, errorText,
      localify::get_localized_string(il2cpp_string_new(message.data())), nullptr, okText);
    errorDialog = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *data,
                                                     bool isEnableOutsideClick)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop", "DialogManager", "PushSystemDialog", 2))(dialogData, true);
}

void *GameSystem_FixedUpdate_orig = nullptr;

void GameSystem_FixedUpdate_hook(Il2CppObject *thisObj) {
    auto sceneManagerField = il2cpp_class_get_field_from_name(thisObj->klass,
                                                              "_sceneManagerInstance");
    il2cpp_field_get_value(thisObj, sceneManagerField, &sceneManager);
    reinterpret_cast<decltype(GameSystem_FixedUpdate_hook) *>(GameSystem_FixedUpdate_orig)(thisObj);
}

void *CriMana_Player_SetFile_orig = nullptr;

bool
CriMana_Player_SetFile_hook(Il2CppObject *thisObj, Il2CppObject *binder, Il2CppString *moviePath,
                            int setMode) {
    stringstream pathStream(localify::u16_u8(moviePath->start_char));
    string segment;
    vector<string> split;
    while (getline(pathStream, segment, '\\')) {
        split.emplace_back(segment);
    }
    if (g_replace_assets.find(split[split.size() - 1]) != g_replace_assets.end()) {
        auto &replaceAsset = g_replace_assets.at(split[split.size() - 1]);
        moviePath = il2cpp_string_new(replaceAsset.path.data());
    }
    return reinterpret_cast<decltype(CriMana_Player_SetFile_hook) *>(CriMana_Player_SetFile_orig)(
            thisObj, binder, moviePath, setMode);
}

void (*SendNotification)(Il2CppObject *thisObj, Il2CppString *ChannelId, Il2CppString *title,
                         Il2CppString *message,
                         DateTime date, Il2CppString *path, int id);

Il2CppString *(*createFavIconFilePath)(Il2CppObject *thisObj, int unitId);

void *GeneratePushNotifyCharaIconPng_orig = nullptr;

Il2CppString *GeneratePushNotifyCharaIconPng_hook(Il2CppObject *thisObj, int unitId, int dressId,
                                                  Boolean  /*forceGen*/) {
    return reinterpret_cast<decltype(GeneratePushNotifyCharaIconPng_hook) * >
    (GeneratePushNotifyCharaIconPng_orig)(
            thisObj, unitId, dressId,
            GetBoolean(true));
}

void *MasterDataManager_ctor_orig = nullptr;

void MasterDataManager_ctor_hook(Il2CppObject *thisObj) {
    reinterpret_cast<decltype(MasterDataManager_ctor_hook) * >
    (MasterDataManager_ctor_orig)(thisObj);
    masterDataManager = thisObj;
}

void
(*RegisterNotificationChannel)(Il2CppObject *thisObj, Il2CppString *name, Il2CppString *ChannelId,
                               Il2CppString *message);

Boolean (*IsDenyTime)(Il2CppObject *thisObj, Il2CppObject *dateTime);

void (*DeleteAllLocalPushes)(Il2CppObject *thisObj);

void *SendNotificationWithExplicitID_orig = nullptr;

void
SendNotificationWithExplicitID_hook(AndroidNotification notificationObj, Il2CppString *channelId,
                                    int id) {
    auto messageJsonIl2CppStr = notificationObj.Text;
    rapidjson::Document document;
    document.Parse(localify::u16_u8(messageJsonIl2CppStr->start_char).data());
    if (!document.HasParseError()) {
        auto message = document["message"].GetString();
        auto largeImgPath = document["largeImgPath"].GetString();
        notificationObj.Text = il2cpp_string_new(message);
        notificationObj.LargeIcon = il2cpp_string_new(largeImgPath);
    }

    reinterpret_cast<decltype(SendNotificationWithExplicitID_hook) * >
    (SendNotificationWithExplicitID_orig)(
            notificationObj, channelId, id);
}

void *ScheduleLocalPushes_orig = nullptr;

void ScheduleLocalPushes_hook(Il2CppObject *thisObj, int type, Il2CppArray *unixTimes,
                              Il2CppArray *values, int  /*_priority*/, Il2CppString * /*imgPath*/) {

    auto charaId = GetInt64Safety(reinterpret_cast<Int64 *>(Array_GetValue(values, 0)));
    if (!masterDataManager) {
        return;
    }
    auto masterString = reinterpret_cast<Il2CppObject *(*)(
            Il2CppObject *thisObj)>(il2cpp_class_get_method_from_name(masterDataManager->klass,
                                                                      "get_masterString",
                                                                      0)->methodPointer)(
            masterDataManager);
    auto cateId = type == 0 ? 184 : 185;
    auto messageIl2CppStrOrig = reinterpret_cast<Il2CppString *(*)(Il2CppObject *, int category,
                                                                   int index)>(
            il2cpp_class_get_method_from_name(masterString->klass, "GetText", 2)->methodPointer
    )(masterString, cateId, (int) charaId);
    // ex. 1841001
    auto messageKey = string(to_string(cateId)).append(
            to_string(charaId));
    auto messageIl2CppStr = localify::get_localized_string(stoi(messageKey));
    if (!messageIl2CppStr) {
        messageIl2CppStr = messageIl2CppStrOrig;
    }

    auto channelId = type == 0 ? "NOTIF_Tp_0" : "NOTIF_Rp_0";
    auto id = type == 0 ? 100 : 200;
    auto typeStr = type == 0 ? "TP" : "RP";
    if (IsDenyTime(thisObj, nullptr).m_value) {
        DeleteAllLocalPushes(thisObj);
        return;
    }
    RegisterNotificationChannel(thisObj,
                                il2cpp_string_new(typeStr),
                                il2cpp_string_new(channelId),
                                il2cpp_string_new(string(typeStr).append(" 알림").data()));
    auto dateTime = FromUnixTimeToLocaleTime(
            GetInt64Safety(reinterpret_cast<Int64 *>(Array_GetValue(unixTimes, 0))));

    auto filePath = createFavIconFilePath(thisObj, charaId);
    rapidjson::Document document(rapidjson::kObjectType);
    rapidjson::Value message;
    rapidjson::Value largeImgPath;
    auto messageStr = localify::u16_u8(messageIl2CppStr->start_char);
    message.SetString(messageStr.data(), messageStr.length());
    auto filePathStr = localify::u16_u8(filePath->start_char);
    largeImgPath.SetString(filePathStr.data(), filePathStr.length());
    document.AddMember("message", message, document.GetAllocator());
    document.AddMember("largeImgPath", largeImgPath, document.GetAllocator());
    rapidjson::StringBuffer buffer;
    buffer.Clear();
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);
    SendNotification(thisObj, il2cpp_string_new(channelId), il2cpp_string_new("우마무스메"),
                     il2cpp_string_new(buffer.GetString()), dateTime,
                     il2cpp_string_new(
                             "icon"),
                     id);
}

void dump_all_entries() {
    vector<u16string> static_entries;
    vector<pair<const string, const u16string>> text_id_static_entries;
    vector<pair<const string, const u16string>> text_id_not_matched_entries;
    // 0 is None
    for (int i = 1;; i++) {
        auto *str = reinterpret_cast<decltype(localize_get_hook) * > (localize_get_orig)(i);

        if (str && *str->start_char) {

            if (g_static_entries_use_text_id_name) {
                const string textIdName = GetTextIdNameById(i);
                text_id_static_entries.emplace_back(
                        textIdName, u16string(str->start_char));
                if (localify::get_localized_string(textIdName) == nullptr ||
                    localify::u16_u8(localify::get_localized_string(textIdName)->start_char) ==
                    localify::u16_u8(str->start_char)) {
                    text_id_not_matched_entries.emplace_back(
                            textIdName, u16string(str->start_char));
                }
            } else if (g_static_entries_use_hash) {
                static_entries.emplace_back(str->start_char);
            } else {
                logger::write_entry(i, str->start_char);
            }
        } else {
            // check next string, if it's still empty, then we are done!
            auto *nextStr = reinterpret_cast<decltype(localize_get_hook) * > (localize_get_orig)(
                    i + 1);
            if (!(nextStr && *nextStr->start_char))
                break;
        }
    }

    if (g_static_entries_use_text_id_name) {
        logger::write_text_id_static_dict(text_id_static_entries, text_id_not_matched_entries);
    } else if (g_static_entries_use_hash) {
        logger::write_static_dict(static_entries);
    }
}

void init_il2cpp_api() {
#define DO_API(r, n, ...) n = (r (*) (__VA_ARGS__))dlsym(il2cpp_handle, #n)

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
            sscanf(line, "%"
                         PRIx64
                         "-%"
                         PRIx64
                         " %s %*"
                         PRIx64
                         " %*x:%*x %*u %s\n", &start, &end,
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
                                                     Il2CppObject *type)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.AssetBundleModule.dll", "UnityEngine",
                    "AssetBundle", "LoadAsset", 2)
    );

    get_all_asset_names = reinterpret_cast<Il2CppArray *(*)(Il2CppObject *thisObj)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.AssetBundleModule.dll", "UnityEngine",
                    "AssetBundle", "GetAllAssetNames", 0)
    );

    uobject_get_name = reinterpret_cast<Il2CppString *(*)(Il2CppObject *uObject)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.CoreModule.dll", "UnityEngine",
                    "Object", "GetName", -1)
    );

    uobject_IsNativeObjectAlive = reinterpret_cast<bool (*)(Il2CppObject *uObject)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.CoreModule.dll", "UnityEngine",
                    "Object", "IsNativeObjectAlive", 1)
    );

    get_unityVersion = reinterpret_cast<Il2CppString *(*)()>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.CoreModule.dll", "UnityEngine",
                    "Application", "get_unityVersion", -1)
    );

    FromUnixTimeToLocaleTime = reinterpret_cast<DateTime (*)(long unixTime)>(
            il2cpp_symbols::get_method_pointer(
                    "umamusume.dll", "Gallop",
                    "TimeUtil", "FromUnixTimeToLocaleTime", 1)
    );

    Array_GetValue = reinterpret_cast<void *(*)(Il2CppArray *, long index)>(
            il2cpp_symbols::get_method_pointer(
                    "mscorlib.dll", "System",
                    "Array", "GetValue", 1)
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

    auto query_setup_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Query", "_Setup", 2
    );

    auto Plugin_sqlite3_step_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Plugin", "sqlite3_step", 1
    );

    auto Plugin_sqlite3_reset_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Plugin", "sqlite3_reset", 1
    );

    auto query_step_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Query", "Step", 0
    );

    auto prepared_query_reset_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "PreparedQuery", "Reset", 0
    );

    auto prepared_query_bind_text_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "PreparedQuery", "BindText", 2
    );

    auto prepared_query_bind_int_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "PreparedQuery", "BindInt", 2
    );

    auto prepared_query_bind_long_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "PreparedQuery", "BindLong", 2
    );

    auto prepared_query_bind_double_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "PreparedQuery", "BindDouble", 2
    );

    auto query_gettext_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Query", "GetText", 1
    );

    query_getint = reinterpret_cast<int (*)(Il2CppObject *,
                                            int)>(il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Query", "GetInt", 1
    ));

    auto query_dispose_addr = il2cpp_symbols::get_method_pointer(
            "LibNative.Runtime.dll", "LibNative.Sqlite3",
            "Query", "Dispose", 0
    );

    auto MasterCharacterSystemText_CreateOrmByQueryResultWithCharacterId_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "MasterCharacterSystemText", "_CreateOrmByQueryResultWithCharacterId", 2
    );

    auto AtomSourceEx_SetParameter_addr = il2cpp_symbols::get_method_pointer(
            "Cute.Cri.Assembly.dll", "Cute.Cri",
            "AtomSourceEx", "SetParameter", 0
    );

    auto CySpringUpdater_set_SpringUpdateMode_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop.Model.Component",
            "CySpringUpdater", "set_SpringUpdateMode", 1
    );

    auto CySpringUpdater_get_SpringUpdateMode_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop.Model.Component",
            "CySpringUpdater", "get_SpringUpdateMode", 0
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

    auto UIManager_OnInitialize_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "UIManager", "OnInitialize", 0
    );

    auto on_populate_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "OnPopulateMesh", 1
    );

    auto textcommon_awake_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "Awake", 0
    );

    auto textcommon_SetSystemTextWithLineHeadWrap_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "SetSystemTextWithLineHeadWrap", 2

    );

    auto textcommon_SetTextWithLineHeadWrapWithColorTag_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "SetTextWithLineHeadWrapWithColorTag", 2

    );

    auto textcommon_SetTextWithLineHeadWrap_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "SetTextWithLineHeadWrap", 2

    );

    auto TextMeshProUguiCommon_Awake_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextMeshProUguiCommon", "Awake", 0
    );

    textcommon_get_TextId = reinterpret_cast<int (*)(
            Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop",
            "TextCommon", "get_TextId", 0
    ));

    text_get_text = reinterpret_cast<Il2CppString *(*)(Il2CppObject *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "get_text", 0
            )
    );
    text_set_text = reinterpret_cast<void (*)(Il2CppObject *, Il2CppString *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_text", 1
            )
    );

    text_assign_font = reinterpret_cast<void (*)(Il2CppObject *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "AssignDefaultFont", 0
            )
    );

    text_set_font = reinterpret_cast<void (*)(Il2CppObject *, Il2CppObject *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_font", 1
            )
    );

    text_get_font = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "get_font", 0
            )
    );

    text_get_size = reinterpret_cast<int (*)(Il2CppObject *)>(
            il2cpp_symbols::get_method_pointer(
                    "umamusume.dll", "Gallop",
                    "TextCommon", "get_FontSize", 0
            )
    );

    text_set_size = reinterpret_cast<void (*)(Il2CppObject *, int)>(
            il2cpp_symbols::get_method_pointer(
                    "umamusume.dll", "Gallop",
                    "TextCommon", "set_FontSize", 1
            )
    );

    text_get_linespacing = reinterpret_cast<float (*)(Il2CppObject *)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "get_lineSpacing", 0
            )
    );

    text_set_style = reinterpret_cast<void (*)(Il2CppObject *, int)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_fontStyle", 1
            )
    );

    text_set_linespacing = reinterpret_cast<void (*)(Il2CppObject *, float)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_lineSpacing", 1
            )
    );

    text_set_horizontalOverflow = reinterpret_cast<void (*)(Il2CppObject *, int)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_horizontalOverflow", 1
            )
    );

    text_set_verticalOverflow = reinterpret_cast<void (*)(Il2CppObject *, int)>(
            il2cpp_symbols::get_method_pointer(
                    "UnityEngine.UI.dll", "UnityEngine.UI",
                    "Text", "set_verticalOverflow", 1
            )
    );

    auto set_fps_addr = il2cpp_symbols::get_method_pointer(
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

    auto set_anti_aliasing_addr = il2cpp_resolve_icall(
            "UnityEngine.QualitySettings::set_antiAliasing(System.Int32)");

    auto Light_set_shadowResolution_addr = il2cpp_resolve_icall(
            "UnityEngine.Light::set_shadowResolution(UnityEngine.Light,UnityEngine.Rendering.LightShadowResolution)");

    display_get_main = reinterpret_cast<Il2CppObject *(*)()>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.CoreModule.dll",
            "UnityEngine",
            "Display", "get_main", -1));

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

    auto GraphicSettings_GetVirtualResolution_addr = reinterpret_cast<Vector2Int_t(*)(
            Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop",
            "GraphicSettings", "GetVirtualResolution", 0));

    auto GraphicSettings_GetVirtualResolution3D_addr = reinterpret_cast<Vector2Int_t(*)(
            Il2CppObject *, bool)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop",
            "GraphicSettings", "GetVirtualResolution3D", 1));

    auto ChangeScreenOrientation_addr = reinterpret_cast<void (*)(
            ScreenOrientation, bool)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop",
            "Screen", "ChangeScreenOrientation", 2));

    auto ChangeScreenOrientationPortraitAsync_addr = reinterpret_cast<Il2CppObject *(*)()>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop", "Screen", "ChangeScreenOrientationPortraitAsync", -1));

    Screen_get_width = reinterpret_cast<int (*)()>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.CoreModule.dll",
            "UnityEngine",
            "Screen", "get_width", -1));

    Screen_get_height = reinterpret_cast<int (*)()>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.CoreModule.dll",
            "UnityEngine",
            "Screen", "get_height", -1));

    auto Screen_set_orientation_addr = reinterpret_cast<void (*)(
            ScreenOrientation)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.CoreModule.dll",
            "UnityEngine",
            "Screen", "set_orientation", 1));

    auto SetResolution_addr = reinterpret_cast<void (*)(int, int, bool,
                                                        bool)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "Screen", "SetResolution", 4));

    auto DeviceOrientationGuide_Show_addr = reinterpret_cast<void (*)(bool,
                                                                      int)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "DeviceOrientationGuide", "Show", 2));

    auto NowLoading_Show_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "NowLoading", "Show", 3);

    auto NowLoading_Show2_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "NowLoading", "Show", 4);

    auto NowLoading_Hide_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "NowLoading", "Hide", 1);

    auto NowLoading_Hide2_addr = il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "NowLoading", "Hide", 3);

    auto WaitDeviceOrientation_addr = reinterpret_cast<void (*)(
            ScreenOrientation)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "Screen", "WaitDeviceOrientation", 1));

    auto CanvasScaler_set_referenceResolution_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                                               float)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.UI.dll",
            "UnityEngine.UI", "CanvasScaler", "set_referenceResolution", 1));

    auto SafetyNet_OnSuccess_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                              float)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "SafetyNet", "OnSuccess", 1));

    auto SafetyNet_OnError_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                            float)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll",
            "Gallop", "SafetyNet", "OnError", 1));

    auto SafetyNet_GetSafetyNetStatus_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                                       float)>(il2cpp_symbols::get_method_pointer(
            "Cute.Core.Assembly.dll",
            "Cute.Core", "SafetyNet", "GetSafetyNetStatus", 4));

    auto Device_IsIllegalUser_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                               float)>(il2cpp_symbols::get_method_pointer(
            "Cute.Core.Assembly.dll",
            "Cute.Core", "Device", "IsIllegalUser", -1));

    MoviePlayerBase_get_MovieInfo = reinterpret_cast<Il2CppObject *(*)(
            Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
            "Cute.Cri.Assembly.dll", "Cute.Cri", "MoviePlayerBase", "get_MovieInfo", 0));

    MovieManager_GetMovieInfo = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *,
                                                                   MoviePlayerHandle)>(il2cpp_symbols::get_method_pointer(
            "Cute.Cri.Assembly.dll", "Cute.Cri", "MovieManager", "GetMovieInfo", 1));

    auto MovieManager_SetImageUvRect_addr = il2cpp_symbols::get_method_pointer(
            "Cute.Cri.Assembly.dll", "Cute.Cri", "MovieManager", "SetImageUvRect", 2);

    auto MovieManager_SetScreenSize_addr = il2cpp_symbols::get_method_pointer(
            "Cute.Cri.Assembly.dll", "Cute.Cri", "MovieManager", "SetScreenSize", 2);


    auto MoviePlayerForUI_AdjustScreenSize_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                                            Vector2_t,
                                                                            bool)>(il2cpp_symbols::get_method_pointer(
            "Cute.Cri.Assembly.dll", "Cute.Cri", "MoviePlayerForUI", "AdjustScreenSize", 2));

    auto MoviePlayerForObj_AdjustScreenSize_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                                             Vector2_t,
                                                                             bool)>(il2cpp_symbols::get_method_pointer(
            "Cute.Cri.Assembly.dll", "Cute.Cri", "MoviePlayerForObj", "AdjustScreenSize", 2));

    auto FrameRateController_OverrideByNormalFrameRate_addr = reinterpret_cast<void (*)(
            Il2CppObject *, int)>(il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop",
                                                                     "FrameRateController",
                                                                     "OverrideByNormalFrameRate",
                                                                     1));

    auto FrameRateController_OverrideByMaxFrameRate_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                                                     int)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop", "FrameRateController", "OverrideByMaxFrameRate", 1));

    auto FrameRateController_ResetOverride_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                                            int)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop", "FrameRateController", "ResetOverride", 1));

    auto FrameRateController_ReflectionFrameRate_addr = reinterpret_cast<void (*)(Il2CppObject *,
                                                                                  int)>(il2cpp_symbols::get_method_pointer(
            "umamusume.dll", "Gallop", "FrameRateController", "ReflectionFrameRate", 0));

    auto GallopUtil_GotoTitleOnError_addr = reinterpret_cast<void (*)(
            Il2CppString *)>(il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop",
                                                                "GallopUtil", "GotoTitleOnError",
                                                                1));

    auto DialogCommon_Close_addr = reinterpret_cast<void (*)(
            Il2CppObject *)>(il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop",
                                                                "DialogCommon", "Close", 0));

    auto GameSystem_FixedUpdate_addr = il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop",
                                                                          "GameSystem",
                                                                          "FixedUpdate", 0);

    auto CriMana_Player_SetFile_addr =
            GetUnityVersion() == Unity2020 ?
            il2cpp_symbols::get_method_pointer(
                    "CriMw.CriWare.Runtime.dll", "CriWare.CriMana", "Player", "SetFile", 3)
                                           :
            il2cpp_symbols::get_method_pointer(
                    "Cute.Cri.Assembly.dll", "CriMana", "Player", "SetFile", 3);

    load_from_file = reinterpret_cast<Il2CppObject *(*)(
            Il2CppString *path)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle", "LoadFromFile", 1));

    /*auto load_from_memory_async = reinterpret_cast<Il2CppObject *(*)(
    Il2CppArray *bytes)>(il2cpp_symbols::get_method_pointer(
    "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle",
    "LoadFromMemoryAsync",
    1));*/

    auto PathResolver_GetLocalPath_addr = reinterpret_cast<Il2CppObject *(*)(
            Il2CppString *path)>(il2cpp_symbols::get_method_pointer(
            "_Cyan.dll", "Cyan.LocalFile", "PathResolver",
            "GetLocalPath", 2));

    auto assetbundle_unload_addr = reinterpret_cast<Il2CppObject *(*)(
            Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle", "Unload", 1));

    auto assetbundle_LoadFromFile_addr = reinterpret_cast<Il2CppObject *(*)(
            Il2CppString *path)>(il2cpp_symbols::get_method_pointer(
            "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle",
            "LoadFromFile", 1));

    if (!assets && !g_font_assetbundle_path.empty() && g_replace_to_custom_font) {
        auto assetbundlePath = localify::u8_u16(g_font_assetbundle_path);
        if (!assetbundlePath.starts_with(u"/")) {
            assetbundlePath.insert(0, u16string(u"/sdcard/Android/data/").append(
                    localify::u8_u16(Game::GetCurrentPackageName())).append(u"/"));
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

    if (assets) {
        LOGI("Asset loaded: %p", assets);
    }

#define ADD_HOOK(_name_) \
    LOGI("ADD_HOOK: %s", #_name_); \
    if (_name_##_addr) DobbyHook(reinterpret_cast<void *>(_name_##_addr), reinterpret_cast<void *>(_name_##_hook), reinterpret_cast<void **>(&_name_##_orig)); \
    else LOGW("ADD_HOOK: %s_addr is null", #_name_);

    if (Game::currentGameRegion == Game::Region::KOR && g_restore_notification && false) {
        SendNotification = reinterpret_cast<void (*)(
                Il2CppObject *, Il2CppString *, Il2CppString *,
                Il2CppString *,
                DateTime, Il2CppString *, int)>(il2cpp_symbols::get_method_pointer("umamusume.dll",
                                                                                   "Gallop",
                                                                                   "PushNotificationManager",
                                                                                   "SendNotification",
                                                                                   6));

        createFavIconFilePath = reinterpret_cast<Il2CppString *(*)(
                Il2CppObject *, int)>(il2cpp_symbols::get_method_pointer("umamusume.dll",
                                                                         "Gallop",
                                                                         "PushNotificationManager",
                                                                         "createFavIconFilePath",
                                                                         1));

        RegisterNotificationChannel = reinterpret_cast<void (*)(
                Il2CppObject *, Il2CppString *, Il2CppString *,
                Il2CppString *)>(il2cpp_symbols::get_method_pointer("umamusume.dll",
                                                                    "Gallop",
                                                                    "PushNotificationManager",
                                                                    "RegisterNotificationChannel",
                                                                    3));

        IsDenyTime = reinterpret_cast<Boolean (*)(
                Il2CppObject *, Il2CppObject *)>(il2cpp_symbols::get_method_pointer("umamusume.dll",
                                                                                    "Gallop",
                                                                                    "PushNotificationManager",
                                                                                    "IsDenyTime",
                                                                                    1));

        DeleteAllLocalPushes = reinterpret_cast<void (*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("umamusume.dll",
                                                                    "Gallop",
                                                                    "PushNotificationManager",
                                                                    "RegisterNotificationChannel",
                                                                    3));

        auto ScheduleLocalPushes_addr = reinterpret_cast<void (*)(
                Il2CppObject *, int)>(il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop",
                                                                         "PushNotificationManager",
                                                                         "ScheduleLocalPushes", 5));

        auto SendNotificationWithExplicitID_addr = reinterpret_cast<void (*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer(
                "Unity.Notifications.Android.dll",
                "Unity.Notifications.Android",
                "AndroidNotificationCenter",
                "SendNotificationWithExplicitID",
                3));

        auto GeneratePushNotifyCharaIconPng_addr = reinterpret_cast<void (*)(
                Il2CppObject *, int, int, Boolean)>(il2cpp_symbols::get_method_pointer(
                "umamusume.dll",
                "Gallop",
                "PushNotificationManager",
                "GeneratePushNotifyCharaIconPng",
                3));

        auto MasterDataManager_ctor_addr = reinterpret_cast<void (*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("umamusume.dll",
                                                                    "Gallop",
                                                                    "MasterDataManager",
                                                                    ".ctor",
                                                                    0));

        ADD_HOOK(MasterDataManager_ctor)

        ADD_HOOK(SendNotificationWithExplicitID)

        ADD_HOOK(GeneratePushNotifyCharaIconPng)

        ADD_HOOK(ScheduleLocalPushes)
    }

    ADD_HOOK(CriMana_Player_SetFile)

    ADD_HOOK(GameSystem_FixedUpdate)

    /*if (GetUnityVersion() == Unity2020) {
        ADD_HOOK(DialogCommon_Close)
    }*/

    // ADD_HOOK(GallopUtil_GotoTitleOnError)

    ADD_HOOK(Light_set_shadowResolution)

    ADD_HOOK(PathResolver_GetLocalPath)

    ADD_HOOK(Device_IsIllegalUser)

    ADD_HOOK(SafetyNet_GetSafetyNetStatus)

    ADD_HOOK(SafetyNet_OnSuccess)

    ADD_HOOK(SafetyNet_OnError)

    if (GetUnityVersion() == Unity2020) {
        ADD_HOOK(NowLoading_Show2)
        if (g_hide_now_loading) {
            ADD_HOOK(NowLoading_Hide2)
        }
    } else {
        ADD_HOOK(NowLoading_Show)
        if (g_hide_now_loading) {
            ADD_HOOK(NowLoading_Hide)
        }
    }

    ADD_HOOK(assetbundle_unload)

    ADD_HOOK(assetbundle_LoadFromFile)

    ADD_HOOK(get_preferred_width)

    ADD_HOOK(an_text_fix_data)

    ADD_HOOK(an_text_set_material_to_textmesh)

    ADD_HOOK(load_zekken_composite_resource)

    ADD_HOOK(UIManager_OnInitialize)

    ADD_HOOK(wait_resize_ui)

    // hook UnityEngine.TextGenerator::PopulateWithErrors to modify text
    ADD_HOOK(populate_with_errors)

    ADD_HOOK(textcommon_SetTextWithLineHeadWrap)
    ADD_HOOK(textcommon_SetTextWithLineHeadWrapWithColorTag)
    ADD_HOOK(textcommon_SetSystemTextWithLineHeadWrap)

    ADD_HOOK(localizeextension_text)

    // Looks like they store all localized texts that used by code in a dict
    ADD_HOOK(localize_get)

    ADD_HOOK(story_timeline_controller_play)

    ADD_HOOK(story_race_textasset_load)

    ADD_HOOK(get_modified_string)

    ADD_HOOK(update)

    ADD_HOOK(query_setup)
    ADD_HOOK(query_gettext)
    ADD_HOOK(query_dispose)

    if (!g_replace_text_db_path.empty()) {
        try {
            replacementMDB = new SQLite::Database(g_replace_text_db_path.data());
            ADD_HOOK(Plugin_sqlite3_step)
            ADD_HOOK(Plugin_sqlite3_reset)
            ADD_HOOK(query_step)
            ADD_HOOK(prepared_query_reset)
            ADD_HOOK(prepared_query_bind_text)
            ADD_HOOK(prepared_query_bind_int)
            ADD_HOOK(prepared_query_bind_long)
            ADD_HOOK(prepared_query_bind_double)
            ADD_HOOK(MasterCharacterSystemText_CreateOrmByQueryResultWithCharacterId)
        } catch (exception &e) {
        }
    }

    if (g_character_system_text_caption) {
        ADD_HOOK(AtomSourceEx_SetParameter)
    }

    if (g_cyspring_update_mode != -1) {
        ADD_HOOK(CySpringUpdater_set_SpringUpdateMode)
        ADD_HOOK(CySpringUpdater_get_SpringUpdateMode)
    }

    if (g_ui_use_system_resolution || g_force_landscape) {
        ADD_HOOK(set_resolution)
    }

    if (g_force_landscape) {
        ADD_HOOK(SetResolution)
        ADD_HOOK(CanvasScaler_set_referenceResolution)
        ADD_HOOK(Screen_set_orientation)
        ADD_HOOK(WaitDeviceOrientation)
        ADD_HOOK(DeviceOrientationGuide_Show)
        ADD_HOOK(ChangeScreenOrientation)
        ADD_HOOK(ChangeScreenOrientationPortraitAsync)
        ADD_HOOK(MovieManager_SetScreenSize)
        ADD_HOOK(MovieManager_SetImageUvRect)
        ADD_HOOK(MoviePlayerForUI_AdjustScreenSize)
        ADD_HOOK(MoviePlayerForObj_AdjustScreenSize)
        ADD_HOOK(GraphicSettings_GetVirtualResolution)
    }

    ADD_HOOK(on_populate)
    if (g_replace_to_builtin_font || g_replace_to_custom_font) {
        ADD_HOOK(textcommon_awake)
        ADD_HOOK(TextMeshProUguiCommon_Awake)
    }

    if (g_max_fps > -1) {
        ADD_HOOK(FrameRateController_OverrideByNormalFrameRate)
        ADD_HOOK(FrameRateController_OverrideByMaxFrameRate)
        ADD_HOOK(FrameRateController_ResetOverride)
        ADD_HOOK(FrameRateController_ReflectionFrameRate)
        ADD_HOOK(set_fps)
    }

    if (g_dump_entries) {
        dump_all_entries();
    }

    if (g_dump_db_entries) {
        logger::dump_db_texts();
    }

    if (g_graphics_quality != -1) {
        ADD_HOOK(apply_graphics_quality)
    }

    if (g_resolution_3d_scale != 1.0f) {
        ADD_HOOK(GraphicSettings_GetVirtualResolution3D)
    }

    if (g_anti_aliasing != -1) {
        ADD_HOOK(set_anti_aliasing)
    }

    LOGI("Unity Version: %s", GetUnityVersion().data());
}

void il2cpp_load_assetbundle() {
    if (!replaceAssets && !g_replace_assetbundle_file_path.empty()) {
        auto assetbundlePath = localify::u8_u16(g_replace_assetbundle_file_path);
        if (!assetbundlePath.starts_with(u"/")) {
            assetbundlePath.insert(0, u16string(u"/sdcard/Android/data/").append(
                    localify::u8_u16(Game::GetCurrentPackageName())).append(u"/"));
        }
        replaceAssets = load_from_file(
                il2cpp_string_new_utf16(assetbundlePath.data(), assetbundlePath.length()));

        if (!replaceAssets && filesystem::exists(assetbundlePath)) {
            LOGI("Replacement AssetBundle founded but not loaded. Maybe Asset BuildTarget is not for Android");
        }
    }

    if (replaceAssets) {
        LOGI("Replacement AssetBundle loaded: %p", replaceAssets);
        auto names = get_all_asset_names(replaceAssets);
        for (int i = 0; i < names->max_length; i++) {
            auto u8Name = localify::u16_u8(
                    static_cast<Il2CppString *>(names->vector[i])->start_char);
            replaceAssetNames.emplace_back(u8Name);
        }

        auto AssetBundleRequest_GetResult_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppString *path)>(il2cpp_symbols::get_method_pointer(
                "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundleRequest",
                "GetResult", 0));

        auto assetbundle_load_asset_addr = reinterpret_cast<Il2CppObject *(*)(Il2CppObject *thisObj,
                                                                              Il2CppString *name,
                                                                              Il2CppObject *runtimeType)>(
                il2cpp_symbols::get_method_pointer(
                        "UnityEngine.AssetBundleModule.dll", "UnityEngine", "AssetBundle",
                        "LoadAsset", 2)
        );

        auto resources_load_addr = reinterpret_cast<Il2CppObject *(*)(Il2CppString *path,
                                                                      Il2CppType *)>(il2cpp_symbols::get_method_pointer(
                "UnityEngine.CoreModule.dll", "UnityEngine", "Resources", "Load", 2));

        auto Sprite_get_texture_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_resolve_icall(
                "UnityEngine.Sprite::get_texture(UnityEngine.Sprite)"));

        auto Renderer_get_material_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Renderer",
                                                                    "get_material", 0));

        auto Renderer_get_materials_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Renderer",
                                                                    "get_materials", 0));

        auto Renderer_get_sharedMaterial_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Renderer",
                                                                    "get_sharedMaterial", 0));

        auto Renderer_get_sharedMaterials_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Renderer",
                                                                    "get_sharedMaterials", 0));

        auto Renderer_set_material_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Renderer",
                                                                    "set_material", 1));

        auto Renderer_set_materials_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Renderer",
                                                                    "set_materials", 1));

        auto Renderer_set_sharedMaterial_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Renderer",
                                                                    "set_sharedMaterial", 1));

        auto Renderer_set_sharedMaterials_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Renderer",
                                                                    "set_sharedMaterials", 1));

        auto Material_get_mainTexture_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Material",
                                                                    "get_mainTexture", 0));

        auto Material_set_mainTexture_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("UnityEngine.CoreModule.dll",
                                                                    "UnityEngine", "Material",
                                                                    "set_mainTexture", 1));

        auto Material_SetTextureI4_addr = il2cpp_symbols::find_method("UnityEngine.CoreModule.dll",
                                                                      "UnityEngine", "Material",
                                                                      [](const MethodInfo *method) {
                                                                          return method->name ==
                                                                                 "SetTexture"s &&
                                                                                 method->parameters->parameter_type->type ==
                                                                                 IL2CPP_TYPE_I4;
                                                                      });

        auto CharaPropRendererAccessor_SetTexture_addr = reinterpret_cast<Il2CppObject *(*)(
                Il2CppObject *)>(il2cpp_symbols::get_method_pointer("umamusume.dll", "Gallop",
                                                                    "CharaPropRendererAccessor",
                                                                    "SetTexture", 1));

        ADD_HOOK(AssetBundleRequest_GetResult)

        ADD_HOOK(assetbundle_load_asset)

        ADD_HOOK(resources_load)

        ADD_HOOK(Sprite_get_texture)

        ADD_HOOK(Renderer_get_material)

        ADD_HOOK(Renderer_get_materials)

        ADD_HOOK(Renderer_get_sharedMaterial)

        ADD_HOOK(Renderer_get_sharedMaterials)

        ADD_HOOK(Renderer_set_material)

        ADD_HOOK(Renderer_set_materials)

        ADD_HOOK(Renderer_set_sharedMaterial)

        ADD_HOOK(Renderer_set_sharedMaterials)

        ADD_HOOK(Material_get_mainTexture)

        ADD_HOOK(Material_set_mainTexture)

        ADD_HOOK(Material_SetTextureI4)

        ADD_HOOK(CharaPropRendererAccessor_SetTexture)
    }

}

void il2cpp_hook_init(void *handle) {
    LOGI("il2cpp_handle: %p", handle);
    il2cpp_handle = handle;
    init_il2cpp_api();
    if (il2cpp_domain_get_assemblies) {
        Dl_info dlInfo;
        if (dladdr(reinterpret_cast<void *>(il2cpp_domain_get_assemblies), &dlInfo)) {
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
