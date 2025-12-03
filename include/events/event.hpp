#pragma once

#include <utility>
#include <variant>

struct InputEvent {
    enum class Type {
        KeyPress,
        KeyRelease,
        KeyRepeat,
        MouseMove,
        MouseClick,
    } type{};

    int key{};

    // x, y represent both cursor positions and scroll offsets.
    float x{};
    float y{};
};

struct ApplicationEvent {
    enum class Type {
        CloseWindow,
        ToggleCursor,
    } type{};
};

struct MovementEvent {
    enum class Type {
        StartMoveForward,
        StopMoveForward,
        StartMoveBackward,
        StopMoveBackward,
        StartMoveLeft,
        StopMoveLeft,
        StartMoveRight,
        StopMoveRight,
        Jump,
        Turn,
    } type{};

    float x{};
    float y{};
};

using EventType = std::variant<InputEvent, ApplicationEvent, MovementEvent>;

class Event {
public:
    EventType event;
    bool handled = false;

    // Helper function to automatically deduce the variant used and call the corresponding constructor.
    // This allows us to directly create an Event with Event::make_event(InputEvent::Type::KeyPress, 1)
    // instead of doing Event{ InputEvent{InputEvent::Type::KeyPress, 1} }.
    template <typename Enum, size_t i = 0, typename ...Args>
    static Event make_event(Enum&& e, Args&&... args) {
        // recursively loop through all variants
        if constexpr (i < std::variant_size_v<EventType>) {
            constexpr bool same_variant = std::is_same_v<
                std::decay_t<decltype(e)>,                                                  // input enum type
                decltype(std::declval<std::variant_alternative_t<i, EventType>>().type)     // enum type of the i-th variant
            >;
            
            if constexpr (same_variant) {
                return Event{ std::variant_alternative_t<i, EventType>{std::forward<Enum>(e), std::forward<Args>(args)...} };
            } else {
                return make_event<Enum, i + 1, Args...>(std::forward<Enum>(e), std::forward<Args>(args)...);
            }
        }
    }

    const char* get_type_name() {
        return std::visit([](auto&& v){ return typeid(v).name(); }, event);
    }
};
