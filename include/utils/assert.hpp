#pragma once

#include "utils/logger.hpp"

#include <exception>

inline void debug_assert(bool condition, string_with_loc error_message) {
#ifdef DEBUG
    if (!condition) {
        std::fprintf(stderr, "[ASSERT FAILED] (%s:%u) ", error_message.loc.file_name(), error_message.loc.line());
        std::fprintf(stderr, "%s\n", error_message.msg.c_str());
        std::terminate();
    }
#endif
}
