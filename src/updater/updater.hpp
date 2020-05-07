#pragma once

#include <cppcoro/task.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/when_all.hpp>

#include <variant>
#include <vector>


template <typename... types>
class updater
{
public:
    updater(uint8_t num_threads) :
        _thread_pool(num_threads)
    {}

    template <typename... vectors>
    updater(uint8_t num_threads, std::tuple<vectors...>& components) :
        _thread_pool(num_threads)
    {
        std::apply([this](auto&... comps) {
            (register_vector(&comps), ...);
        }, components);
    }

    updater(const updater&) = delete;
    updater(updater&&) = default;

    template <typename... Args>
    cppcoro::task<> update(Args&&... args)
    {
        std::vector<cppcoro::task<>> tasks;

        for (auto& variant : _vectors)
        {
            std::visit([this, &tasks, ...args { std::forward<Args>(args) }](auto& vec) mutable {
                using E = typename std::remove_pointer<std::decay_t<decltype(vec)>>::type;

                if constexpr (!E::derived_t::template is_updatable<Args...>())
                {
                    return;
                }
                else
                {
                    tasks.push_back(update_impl(vec, std::forward<Args>(args)...));
                }
                
            }, variant);
        }

        co_await cppcoro::when_all(std::move(tasks));
        co_return;
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

    cppcoro::static_thread_pool& thread_pool()
    {
        return _thread_pool;
    }

private:
    template <typename T, typename... Args>
    cppcoro::task<> update_impl(T* vector, Args&&... args)
    {
        co_await _thread_pool.schedule();
        
        for (auto obj : vector->range())
        {
            obj->base()->update(std::forward<Args>(args)...);
        }

        co_return;
    }

private:
    cppcoro::static_thread_pool _thread_pool;
    std::vector<std::variant<types...>> _vectors;
};
