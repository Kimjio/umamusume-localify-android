#include "../stdinclude.hpp"

#include "localify.h"
#include "../../log.h"
#include "../../logger/logger.h"
#include <codecvt>

using namespace std;
using namespace logger;

namespace localify {
    namespace {
        unordered_map<size_t, string> text_db;
        unordered_map<string, string> textId_text_db;
        std::vector<size_t> str_list;
    }

    u16string u8_u16(const string &u8) {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        return utf16conv.from_bytes(u8);
    }

    string u16_u8(const u16string &u16) {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        return utf16conv.to_bytes(u16);
    }

    wstring u8_wide(const string &mbs) {
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wconv;
        return wconv.from_bytes(mbs);
    }

    wstring u16_wide(const u16string &str) {
        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> utf16conv;
        std::string utf8 = utf16conv.to_bytes(str);
        return u8_wide(utf8);
    }

    string wide_u8(const wstring &wstr) {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> conversion;
        std::string mbs = conversion.to_bytes(wstr);
        return mbs;
    }

    void load_textdb(const string &version, const vector<string> *dicts) {
        string path = string("/sdcard/Android/data/").append(Game::GetCurrentPackageName()).append(
                "/");
        for (const auto &dict: *dicts) {
            if (filesystem::exists(path + dict)) {
                std::ifstream dict_stream{path + dict};

                if (!dict_stream.is_open()) {
                    LOGW("Cannot Open File: %s", (path + dict).data());
                    continue;
                }

                LOGI("Reading %s...\n", dict.data());

                rapidjson::IStreamWrapper wrapper{dict_stream};

                rapidjson::Document document;

                document.ParseStream(wrapper);
                bool enabled = true;
                if (document.HasMember("_enabled")) {
                    enabled = document["_enabled"].GetBool();
                    document.EraseMember("_enabled");
                }
                string supportVersion;
                if (document.HasMember("_supportVersion")) {
                    supportVersion = document["_supportVersion"].GetString();
                    document.EraseMember("_supportVersion");
                }
                bool support = true;
                if (!supportVersion.empty() && !version.starts_with(supportVersion)) {
                    support = false;
                }
                if (enabled && support) {
                    for (auto iter = document.MemberBegin();
                         iter != document.MemberEnd(); ++iter) {
                        auto key = std::stoull(iter->name.GetString());
                        auto value = iter->value.GetString();

                        text_db.emplace(key, value);
                    }
                }

                dict_stream.close();
            } else {
                LOGW("File not found: %s", (path + string(dict)).data());
            }
        }
        LOGI("loaded %lu localized entries.\n", text_db.size() + 0l);
    }

    void load_textId_textdb(const std::string &dict) {
        string path = string("/sdcard/Android/data/").append(Game::GetCurrentPackageName()).append(
                "/");
        if (filesystem::exists(path + dict)) {
            std::ifstream dict_stream{path + dict};

            if (!dict_stream.is_open()) {
                LOGW("Cannot Open File: %s", (path + dict).data());
                return;
            }

            printf("Reading %s...\n", dict.data());

            rapidjson::IStreamWrapper wrapper{dict_stream};
            rapidjson::Document document;

            document.ParseStream(wrapper);

            for (auto iter = document.MemberBegin();
                 iter != document.MemberEnd(); ++iter) {
                auto key = iter->name.GetString();
                auto value = iter->value.GetString();

                textId_text_db.emplace(key, value);
            }

            dict_stream.close();
        }

        LOGI("loaded %lu TextId localized entries.", textId_text_db.size() + 0l);
    }

    /*void reload_textdb(const vector<string> *dicts) {
        text_db.clear();
        load_textdb(dicts);
    }*/

    bool localify_text(size_t hash, string **result) {
        if (text_db.contains(hash)) {
            *result = &text_db[hash];
            return true;
        }

        return false;
    }

    bool localify_text_by_textId_name(const string &textIdName, string **result) {
        if (textId_text_db.contains(textIdName)) {
            *result = &textId_text_db[textIdName];
            return true;
        }
        return false;
    }

    Il2CppString *get_localized_string(size_t hash_or_id) {
        string *result;

        if (localify::localify_text(hash_or_id, &result)) {
            return il2cpp_string_new(result->data());
        }

        return nullptr;
    }

    Il2CppString *get_localized_string(Il2CppString *str) {
        string *result;

        auto hash = fnv1a::fnv1a_hash_bytes(str->start_char, u16string(str->start_char).length());

        if (localify::localify_text(hash, &result)) {
            return il2cpp_string_new(result->data());
        }

        if (g_enable_logger && !std::any_of(str_list.begin(), str_list.end(),
                                            [hash](size_t hash1) { return hash1 == hash; })) {
            str_list.push_back(hash);

            logger::write_entry(hash, str->start_char);
        }

        return str;
    }

    Il2CppString *get_localized_string(const string &textIdName) {
        string *result;

        if (localify::localify_text_by_textId_name(textIdName, &result)) {
            return il2cpp_string_new(result->data());
        }

        return nullptr;
    }
}
