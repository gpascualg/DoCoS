#pragma once

#include "traits/shared_function.hpp"
#include "updater/updater.hpp"

#include <cppcoro/multi_producer_sequencer.hpp>
#include <cppcoro/static_thread_pool.hpp>
#include <cppcoro/sync_wait.hpp>

#include <iostream>
#include <tuple>


class executor
{
public:
    static inline executor* instance = nullptr;
    static constexpr std::size_t buffer_size = 1024;

public:
    executor() :
        _sequencer(_barrier, buffer_size),
        _next_to_read(0),
        _current_id(0)
    {
        // What if there is more than one executor?
        instance = this;
    }

    template <typename C>
    cppcoro::task<> schedule(C&& callback) 
    {
        auto seq = co_await _sequencer.claim_one(_thread_pool);
        _buffer[seq & (buffer_size - 1)] = make_shared_function(std::move(callback));
        _sequencer.publish(seq);
        co_return;
    }

    template <typename U, typename... Args>
    void update(U& updater, Args&&... args)
    {
        cppcoro::sync_wait(updater.update(std::forward<Args>(args)...));
    }

    void execute_tasks()
    {
        if (_next_to_read <=  _sequencer.last_published_after(_next_to_read - 1))
        {
            std::size_t available = _sequencer.last_published_after(_next_to_read - 1);
            do
            {
                _buffer[_next_to_read & (buffer_size - 1)]();
            } while (_next_to_read++ != available);

            // Notify that we've finished processing up to 'available'.
            _barrier.publish(available);
        }
    }

    template <typename U, typename... Args>
    void draw(U& updater, Args&&... args)
    {
        updater.draw(std::forward<Args>(args)...);
    }

    template <template <typename...> typename S, typename... A, typename... vecs>
    constexpr uint64_t create(S<vecs...>& scheme, A&&... scheme_args)
    {
        return create_with_callback(scheme, [](auto&&... e) { return std::tuple(e...); }, scheme_args...);
    }

    template <template <typename...> typename S,typename... Args, typename... vecs>
    constexpr uint64_t create_with_args(S<vecs...>& scheme, Args&&... args)
    {
        return create_with_callback(scheme, [](auto&&... e) { return std::tuple(e...); }, 
            scheme.template args<vecs>(std::forward<Args>(args)...)...
        );
    }

    template <template <typename...> typename S, typename C, typename... A, typename... vecs>
    constexpr uint64_t create_with_callback(S<vecs...>& scheme, C&& callback, A&&... scheme_args)
    {
        static_assert(sizeof...(vecs) == sizeof...(scheme_args), "Incomplete scheme creation");

        uint64_t id = next_id();
        
        cppcoro::sync_wait(schedule([id, callback { std::move(callback) }, ...scheme_args { std::move(scheme_args) }, &scheme] {
            auto entities = callback(std::apply([&](auto&&... args) {
                auto component = scheme_args.comp.alloc(id, std::forward<decltype(args)>(args)...);
                component->base()->scheme_information(scheme);
                return component;
            }, std::move(scheme_args.args))...);

            std::apply([](auto&&... entities) {
                (..., entities->base()->scheme_created());
            }, std::move(entities));
        }));

        return id;
    }

private:
    uint64_t next_id()
    {
        if (!_free_ids.empty())
        {
            auto id = _free_ids.back();
            _free_ids.pop_back();
            return id;
        }
        return _current_id++;
    }

private:
    cppcoro::static_thread_pool _thread_pool;
    cppcoro::sequence_barrier<std::size_t> _barrier;
    cppcoro::multi_producer_sequencer<std::size_t> _sequencer;
    std::function<void()> _buffer[buffer_size];
    std::size_t _next_to_read;

    uint64_t _current_id;
    std::vector<uint64_t> _free_ids;
};
