#include "events/event_manager.hpp"
#include "graphics/common.hpp"
#include "utils/logger.hpp"

void EventManager::queue_event(Event event) {
    events.push(std::move(event));
}

void EventManager::process_events(std::queue<Event>& app_event_queue) {
    while (!events.empty()) {
        Event event = std::move(events.front());
        events.pop();
        const char* event_type_name = event.get_type_name();

        log_debug("Event type: %s, handled: %d", event.get_type_name(), event.handled);

        if (event_system_map.contains(event_type_name)) {
            for (System* system : event_system_map.at(event_type_name)) {
                system->add_event(event);
            }
        }
        if (std::holds_alternative<ApplicationEvent>(event.event)) {
            app_event_queue.push(std::move(event));
        }
    }
}

void EventManager::queue_input_event(Event event) {
    if (InputEvent* input_event = std::get_if<InputEvent>(&event.event)) {

        // TODO: change to some kind of map/config file
        if (input_event->type == InputEvent::Type::KeyPress) {
            switch (input_event->key) {
                case GLFW_KEY_W:
                    queue_event(Event::make_event(MovementEvent::Type::StartMoveForward));
                    break;
                case GLFW_KEY_A:
                    queue_event(Event::make_event(MovementEvent::Type::StartMoveBackward));
                    break;
                case GLFW_KEY_S:
                    queue_event(Event::make_event(MovementEvent::Type::StartMoveLeft));
                    break;
                case GLFW_KEY_D:
                    queue_event(Event::make_event(MovementEvent::Type::StartMoveRight));
                    break;
                case GLFW_KEY_E:
                    queue_event(Event::make_event(ApplicationEvent::Type::ToggleCursor));
                    break;
                case GLFW_KEY_ESCAPE:
                    queue_event(Event::make_event(ApplicationEvent::Type::CloseWindow));
                    break;
                default:
                    break;
            }
        } else if (input_event->type == InputEvent::Type::KeyRelease) {
            switch (input_event->key) {
                case GLFW_KEY_W:
                    queue_event(Event::make_event(MovementEvent::Type::StopMoveForward));
                    break;
                case GLFW_KEY_A:
                    queue_event(Event::make_event(MovementEvent::Type::StopMoveBackward));
                    break;
                case GLFW_KEY_S:
                    queue_event(Event::make_event(MovementEvent::Type::StopMoveLeft));
                    break;
                case GLFW_KEY_D:
                    queue_event(Event::make_event(MovementEvent::Type::StopMoveRight));
                    break;
                default:
                    break;
            }
        }
    }
}
