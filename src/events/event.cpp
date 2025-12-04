#include "events/event.hpp"

const char* Event::get_type_name() {
    return std::visit([](auto&& v){ return typeid(v).name(); }, event);
}
