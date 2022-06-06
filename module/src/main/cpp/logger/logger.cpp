//
// Created by kimji on 2022-05-01.
//

#include <thread>
#include <utility>
#include "../stdinclude.hpp"
#include "../localify/localify.h"
#include "../game.hpp"
#include <rapidjson/rapidjson.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include "../sqlite3/sqlite3.h"

using namespace std;
using namespace localify;
using namespace rapidjson;

namespace logger {
    fstream log_file;

    bool enabled = false;
    bool request_exit = false;
    bool has_change = false;

    // copy-pasted from https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
    void replaceAll(std::string &str, const std::string &from, const std::string &to) {
        if (from.empty())
            return;
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
    }


    void init_logger() {
        // only output if file exists so regular user will not see it.
        if (g_enable_logger) {
            enabled = true;
            log_file.open(
                    string("/sdcard/Android/data/").append(GamePackageName).append("/dump.txt"),
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

    void dump_db_text(sqlite3 *db, const string &tableName, const string &targetField,
                      const string &jsonPath) {
        string jsonFolderPath = string("/sdcard/Android/data/").append(GamePackageName).append(
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

        sqlite3_stmt *stmt;

        sqlite3_prepare_v2(db, string("SELECT ").append(targetField).append(" FROM ").append(
                tableName).data(), -1, &stmt, nullptr);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            u16string u16str = (char16_t *) sqlite3_column_text16(stmt, 0);
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
        string path = string("/data/data/").append(GamePackageName).append(
                "/files/master/master.mdb");
        if (filesystem::exists(path)) {
            sqlite3 *db;
            int resultCode;
            resultCode = sqlite3_open(path.data(), &db);

            if (resultCode != SQLITE_OK) {
                LOGW("master.mdb not found.");
            }

            dump_db_text(db, "text_data", "text", "common.json");
            dump_db_text(db, "character_system_text", "text", "chara.json");
            dump_db_text(db, "race_jikkyo_comment", "message", "race_comment.json");
            dump_db_text(db, "race_jikkyo_message", "message", "race_message.json");
            sqlite3_close(db);
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
