#ifndef UMAMUSUMELOCALIFYANDROID_LOGGER_H
#define UMAMUSUMELOCALIFYANDROID_LOGGER_H

#pragma once

namespace logger {
    void init_logger();

    void close_logger();

    void dump_db_texts();

    void write_entry(size_t hash, const std::u16string &text);

    void write_static_dict(const std::vector<std::u16string> &dict);

    void write_text_id_static_dict(
            const std::vector<std::pair<const std::string, const std::u16string>> &dict,
            const std::vector<std::pair<const std::string, const std::u16string>> &not_matched);
}

#endif //UMAMUSUMELOCALIFYANDROID_LOGGER_H
