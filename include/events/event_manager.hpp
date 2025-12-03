#pragma once

#include "ecs/system.hpp"
#include "events/common.hpp"

#include <queue>
#include <unordered_map>

class EventManager {
public:
    void queue_event(Event event);
    void process_events(std::queue<Event>& app_event_queue);

    // Maps InputEvents to their corresponding Events to be put on the event queue.
    void queue_input_event(Event event);

    // Once a System is subscribed to an Event, the Event manager will send a copy of the Event
    // to the System in process_events() every time such an Event is received.
    template <typename E>
    void subscribe_system_to_event(System* system) {
        const char* event_type_name = typeid(E).name();

        if (!event_system_map.contains(event_type_name)) {
            event_system_map.insert({event_type_name, {system}});
        } else {
            event_system_map.at(event_type_name).push_back(system);
        }
    }

private:
    // Both EventManager and System are managed by ClientApplication, so EventManager will not outlive System?
    std::unordered_map<const char*, std::vector<System*>> event_system_map{};
    std::queue<Event> events{};
};
