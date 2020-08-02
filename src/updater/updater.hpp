#pragma once

#include "fiber/exclusive_work_stealing.hpp"

#include <boost/fiber/fiber.hpp>
#include <boost/fiber/mutex.hpp>
#include <boost/fiber/condition_variable.hpp>

#include <variant>
#include <vector>


template <typename... types>
class updater
{
public:
    updater(uint8_t num_threads)
    {}

    template <typename... vectors>
    updater(uint8_t num_threads, std::tuple<vectors...>& components)
    {
        std::apply([this](auto&... comps) {
            (register_vector(&comps), ...);
        }, components);
    }

    updater(const updater&) = delete;
    updater(updater&&) = default;

    template <typename... Args>
    void update(Args&&... args)
    {
        for (auto& variant : _vectors)
        {
            std::visit([this, ...args { std::forward<Args>(args) }](auto& vec) mutable {
                using E = typename std::remove_pointer<std::decay_t<decltype(vec)>>::type;

                if constexpr (!E::derived_t::template is_updatable<Args...>())
                {
                    return;
                }
                else
                {
                    boost::fibers::fiber([this, &vec, ...args{ std::forward<Args>(args) }]() {
                        update_impl(vec, std::forward<Args>(args)...);
                    }).detach();
                }
                
            }, variant);
        }
    }

    template <typename... Args>
    void draw(Args&&... args)
    {
        for (auto& variant : _vectors)
        {
            std::visit([this, ...args { std::forward<Args>(args) }](auto& vec) mutable {
                using E = typename std::remove_pointer<std::decay_t<decltype(vec)>>::type;

                if constexpr (!E::derived_t::template is_drawable<Args...>())
                {
                    return;
                }
                else
                {
                    for (auto obj : vec->range())
                    {
                        obj->base()->draw(std::forward<Args>(args)...);
                    }
                }
            }, variant);
        }
    }

    template <typename T>
    void register_vector(T* vector)
    {
        _vectors.emplace_back(vector);
    }

    template <typename T>
    bool unregister_vector(T* vector)
    {
        for (auto it = _vectors.begin(); it != _vectors.end(); ++it)
        {
            auto& variant = *it;

            if (std::holds_alternative<T*>(variant))
            {
                auto& vec = std::get<T*>(variant);
                if (vec == vector)
                {
                    _vectors.erase(it);
                    return true;
                }
            }
        }

        return false;
    }

private:
    template <typename T, typename... Args>
    void update_impl(T* vector, Args&&... args)
    {
        // TODO(gpascualg): Make this a vector member so that we don't initialize/destroy every time
        uint64_t pending_updates = vector->size();
        boost::fibers::mutex updates_mutex;
        boost::fibers::condition_variable_any updates_cv;

        reinterpret_cast<exclusive_work_stealing<0>*>(get_scheduling_algorithm().get())->start_bundle();

        for (auto obj : vector->range())
        {

            boost::fibers::fiber([obj, ...args{ std::forward<Args>(args) }, &updates_mutex, &updates_cv]() mutable {
                obj->base()->update(std::forward<Args>(args)...);

                updates_mutex.lock();
                --pending_updates;
                updates_mutex.unlock();

                if (_pending_updates == 0)
                {
                    _updates_cv.notify_all();
                }
            }).detach();
        }

        reinterpret_cast<exclusive_work_stealing<0>*>(get_scheduling_algorithm().get())->end_bundle();

        updates_mutex.lock();
        updates_cv.wait(_updates_mutex, [&pending_updates]() { return pending_updates == 0; });
        updates_mutex.unlock();
    }

private:
    std::vector<std::variant<types...>> _vectors;
};
