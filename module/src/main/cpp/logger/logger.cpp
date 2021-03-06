//
// Created by kimji on 2022-05-01.
//

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

    bool enabled = false;
    bool request_exit = false;
    bool has_change = false;

    void init_logger() {
        // only output if file exists so regular user will not see it.
        if (g_enable_logger) {
            enabled = true;
            log_file.open(
                    string("/sdcard/Android/data/").append(GetCurrentPackageName()).append(
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
                GetCurrentPackageName()).append(
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
        string path = string("/data/data/").append(GetCurrentPackageName()).append(
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
}
