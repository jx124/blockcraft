#pragma once

#include <cstdio>
#include <source_location>
#include <string>

struct string_with_loc {
    std::string msg;
    std::source_location loc;

    template <typename T>
    string_with_loc(T&& format_string, std::source_location loc = std::source_location::current())
        : msg(std::forward<T>(format_string)), loc(loc) {}
};

template <typename... Args>
void log_debug(string_with_loc format_string, Args&&... args) {
#if DEBUG
    std::printf("[DEBUG] ");
    std::printf(format_string.msg.c_str(), args...);
    std::printf("\n");
#else
    (void)format_string;
    (..., (void)args);
#endif
}

template <typename... Args>
void log_debug_line(string_with_loc format_string, Args&&... args) {
#if DEBUG
    std::printf("[DEBUG] (%s:%u) ", format_string.loc.file_name(), format_string.loc.line());
    std::printf(format_string.msg.c_str(), args...);
    std::printf("\n");
#else
    (void)format_string;
    (..., (void)args);
#endif
}

template <typename... Args>
void log_error(string_with_loc format_string, Args&&... args) {
    std::fprintf(stderr, "[ERROR] (%s:%u) ", format_string.loc.file_name(), format_string.loc.line());
    std::fprintf(stderr, format_string.msg.c_str(), args...);
    std::fprintf(stderr, "\n");
}
