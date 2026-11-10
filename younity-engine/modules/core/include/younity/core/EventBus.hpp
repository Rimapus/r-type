/*
** EPITECH PROJECT, 2026
** younity-engine/modules/core/include/younity/core/EventBus
** File description:
** EventBus header file
*/

#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

namespace younity {

/**
 * @brief Lets parts of the engine and of the game talk without knowing each other:
 *        some publish events, others subscribe functions called with each published event.
 */
class EventBus
{
public:
    /**
     * @brief Call a function each time an event of this type is published.
     *
     * @tparam Event The type of the event, e.g. subscribe<CollisionEvent>(...)
     * @param callback The function to call, receiving the published event
     */
    template <typename Event>
    void subscribe(std::function<void(const Event &)> callback)
    {
        this->getSubscribers<Event>().callbacks.push_back(std::move(callback));
    }

    /**
     * @brief Call all the functions subscribed to this type of event, in the order they subscribed.
     *
     * @tparam Event The type of the event
     * @param event The event given to the subscribers
     */
    template <typename Event>
    void publish(const Event &event)
    {
        const std::vector<std::function<void(const Event &)>> callbacks = this->getSubscribers<Event>().callbacks;

        for (const std::function<void(const Event &)> &callback : callbacks) {
            callback(event);
        }
    }

private:
    /**
     * @brief Base of the subscriber lists, whatever their event type.
     */
    class ISubscribers
    {
    public:
        virtual ~ISubscribers(void) = default;
    };

    /**
     * @brief The functions subscribed to one type of event.
     *
     * @tparam Event The type of the event
     */
    template <typename Event>
    struct Subscribers : public ISubscribers
    {
        std::vector<std::function<void(const Event &)>> callbacks;
    };

    /**
     * @brief Get the subscribers of an event type, created if they do not exist yet.
     *
     * @tparam Event The type of the event
     *
     * @return The functions subscribed to this type of event.
     */
    template <typename Event>
    [[nodiscard]] Subscribers<Event> &getSubscribers(void)
    {
        std::unique_ptr<ISubscribers> &subscribers = this->_subscribers[std::type_index(typeid(Event))];

        if (subscribers == nullptr) {
            subscribers = std::make_unique<Subscribers<Event>>();
        }
        return static_cast<Subscribers<Event> &>(*subscribers);
    }

    std::unordered_map<std::type_index, std::unique_ptr<ISubscribers>> _subscribers;
};

} // namespace younity
