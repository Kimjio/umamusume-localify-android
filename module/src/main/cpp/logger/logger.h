#ifndef UMAMUSUMELOCALIFYANDROID_LOGGER_H
#define UMAMUSUMELOCALIFYANDROID_LOGGER_H

#pragma once

namespace logger {
    void init_logger();

    void close_logger();

    void dump_db_texts();

    void write_entry(size_t hash, const std::u16string &text);

    void write_static_dict(const std::vector<const std::u16string>& dict);
}

#endif //UMAMUSUMELOCALIFYANDROID_LOGGER_H
