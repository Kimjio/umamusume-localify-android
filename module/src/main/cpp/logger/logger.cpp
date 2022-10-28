#include <thread>
#include <utility>
#include "../stdinclude.hpp"
#include "../localify/localify.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <SQLiteCpp/SQLiteCpp.h>

using namespace std;
using namespace localify;
using namespace rapidjson;

namespace logger {
    fstream log_file;
    fstream static_json;
    fstream not_matched_json;

    bool enabled = false;
    bool request_exit = false;
    bool has_change = false;

    void init_logger() {
        // only output if file exists so regular user will not see it.
        if (g_enable_logger) {
            enabled = true;
            log_file.open(
                    string("/sdcard/Android/data/").append(Game::GetCurrentPackageName()).append(
                            "/dump.txt"),
                    ios::app | ios::out);

            thread t([]() {
                while (!request_exit) {
                    this_thread::sleep_for(1s);

                    if (has_change) {
                        log_file.flush();
                        has_change = false;
                    }
                }

                log_file.close();
            });

            t.detach();
        }
    }

    void close_logger() {
        request_exit = true;
    }

    void dump_db_text(SQLite::Database *db, const string &tableName, const string &targetField,
                      const string &jsonPath) {
        string jsonFolderPath = string("/sdcard/Android/data/").append(
                Game::GetCurrentPackageName()).append(
                "/").append(
                "original_data/");
        if (!filesystem::exists(jsonFolderPath)) {
            filesystem::create_directories(
                    jsonFolderPath);
        }

        std::ifstream dict_stream{jsonFolderPath + jsonPath};

        if (!dict_stream.is_open()) {
            LOGW("Cannot Open File: %s", (jsonFolderPath + jsonPath).data());
        }

        rapidjson::IStreamWrapper wrapper{dict_stream};
        rapidjson::Document document;

        document.ParseStream(wrapper);
        if (document.IsNull()) {
            document.SetObject();
        }

        dict_stream.close();

        SQLite::Statement query(*db, string("SELECT ").append(targetField).append(" FROM ").append(
                tableName));

        while (query.executeStep()) {
            u16string u16str = u8_u16(query.getColumn(0).getString());
            wstring text = u16_wide(u16str);
            auto hash = fnv1a::fnv1a_hash_bytes(u16str.data(), u16str.length());
            if (!document.HasMember(to_string(hash).data())) {
                Value key;
                key.SetString(to_string(hash).data(), document.GetAllocator());
                Value textV;
                textV.SetString(localify::wide_u8(text).data(), document.GetAllocator());
                document.AddMember(key, textV, document.GetAllocator());
            }
        }

        std::ofstream json_stream{jsonFolderPath + jsonPath};
        OStreamWrapper osw(json_stream);

        PrettyWriter<OStreamWrapper> writer(osw);
        document.Accept(writer);
        json_stream.close();
    }

    void dump_db_texts() {
        string path = string("/data/data/").append(Game::GetCurrentPackageName()).append(
                "/files/master/master.mdb");
        if (filesystem::exists(path)) {
            try {
                SQLite::Database db(path);

                dump_db_text(&db, "text_data", "text", "common.json");
                dump_db_text(&db, "character_system_text", "text", "chara.json");
                dump_db_text(&db, "race_jikkyo_comment", "message", "race_comment.json");
                dump_db_text(&db, "race_jikkyo_message", "message", "race_message.json");
            } catch (SQLite::Exception &e) {
                LOGW("master.mdb not found.");
            }
        }
    }

    void write_entry(size_t hash, const u16string &text) {
        if (!enabled)
            return;

        auto u8str = localify::wide_u8(localify::u16_wide(text));
        replaceAll(u8str, "\n", "\\n");
        replaceAll(u8str, "\"", "\\\"");

        log_file << "\"" << hash << "\": \"" << u8str << "\",\n";

        has_change = true;
    }

    void write_static_dict(const vector<u16string> &dict) {
        if (g_enable_logger) {
            string jsonPath = string("/sdcard/Android/data/").append(
                    Game::GetCurrentPackageName()).append(
                    "/static.json");
            static_json.open(jsonPath, ios::out);
            static_json << "{\n";
            thread t([dict]() {
                for (int i = 0; i < dict.size(); i++) {
                    auto hash = std::hash<u16string>{}(dict[i]);
                    auto u8str = localify::u16_u8(dict[i]);
                    replaceAll(u8str, "\n", "\\n");
                    replaceAll(u8str, "\"", "\\\"");
                    if (i == dict.size() - 1) {
                        static_json << "\"" << hash << "\": \"" << u8str << "\"\n";
                    } else {
                        static_json << "\"" << hash << "\": \"" << u8str << "\",\n";
                    }
                }
                static_json << "}\n";
                static_json.close();
            });

            t.detach();
        }
    }

    void write_text_id_static_dict(const vector<pair<const string, const u16string>> &dict,
                                   const vector<pair<const string, const u16string>> &not_matched) {
        if (g_enable_logger) {
            string jsonPath = string("/sdcard/Android/data/").append(
                    Game::GetCurrentPackageName()).append(
                    "/");
            static_json.open(jsonPath + "text_id_static.json", ios::out);
            static_json << "{\n";
            thread t([dict]() {
                for (auto pair = dict.begin(); pair != dict.end(); pair++) {
                    auto u8str = localify::u16_u8(pair->second);
                    replaceAll(u8str, "\n", "\\n");
                    replaceAll(u8str, "\"", "\\\"");
                    if (next(pair) == dict.end()) {
                        static_json << "\"" << pair->first << "\": \"" << u8str << "\"\n";
                    } else {
                        static_json << "\"" << pair->first << "\": \"" << u8str << "\",\n";
                    }
                }
                static_json << "}\n";
                static_json.close();
            });

            t.detach();
            if (!not_matched.empty()) {
                not_matched_json.open(jsonPath + "text_id_not_matched.json", ios::out);
                not_matched_json << "{\n";
                thread t1([not_matched]() {
                    for (auto pair = not_matched.begin(); pair != not_matched.end(); pair++) {
                        auto u8str = localify::u16_u8(pair->second);
                        replaceAll(u8str, "\n", "\\n");
                        replaceAll(u8str, "\"", "\\\"");
                        if (next(pair) == not_matched.end()) {
                            not_matched_json << "\"" << pair->first << "\": \"" << u8str << "\"\n";
                        } else {
                            not_matched_json << "\"" << pair->first << "\": \"" << u8str << "\",\n";
                        }
                    }
                    not_matched_json << "}\n";
                    not_matched_json.close();
                });

                t1.detach();
            }
        }
    }
}
