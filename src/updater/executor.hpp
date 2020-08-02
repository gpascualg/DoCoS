#pragma once

#include "containers/thread_local_tasks.hpp"
#include "traits/shared_function.hpp"
#include "updater/updater.hpp"

#include <boost/fiber/fiber.hpp>
#include <boost/fiber/barrier.hpp>

#include <iostream>
#include <tuple>


class executor
{
public:
    static inline executor* instance = nullptr;
    static constexpr std::size_t buffer_size = 1024;

public:
    executor(uint8_t num_workers) :
        _threads_joined(num_workers),
        _current_id(0)
    {
        // What if there is more than one executor?
        instance = this;

        boost::fibers::barrier threads_ready(num_workers);

        for (uint8_t thread_id = 1; thread_id < num_workers; ++thread_id)
        {

            _workers.push_back(std::thread(
                [this, num_workers, &threads_ready] {
                    // Set thread algo
                    public_scheduling_algorithm<exclusive_work_stealing<0>>(num_workers);

                    // Signal ready
                    threads_ready.wait();

                    _mutex.lock();
                    // suspend main-fiber from the worker thread
                    _cv.wait(_mutex);
                    _mutex.unlock();

                    // Wait all
                    _threads_joined.wait();
                })
            );
        }

        // Set thread algo
        public_scheduling_algorithm<exclusive_work_stealing<0>>(num_workers);

        // Wait for others
        threads_ready.wait();
    }

    void stop()
    {
        // Notify, wait and join workers
        _cv.notify_all();
        _threads_joined.wait();

        for (auto& t : _workers)
        {
            t.join();
        }
    }

    template <typename C>
    void schedule(C&& callback) 
    {
        get_scheduler().schedule(make_shared_function(std::move(callback))); // ;
    }

    template <typename U, typename... Args>
    void update(U& updater, Args&&... args)
    {
        updater.update(std::forward<Args>(args)...);
    }

    void execute_tasks()
    {
        for (auto ts : thread_local_storage<tasks>::get())
        {
            ts->execute();
        }

        for (auto ts : thread_local_storage<async_tasks>::get())
        {
            ts->execute();
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
        
        schedule([id, callback { std::move(callback) }, ...scheme_args { std::move(scheme_args) }, &scheme] {
            auto entities = callback(std::apply([&](auto&&... args) {
                auto component = scheme_args.comp.alloc(id, std::forward<decltype(args)>(args)...);
                component->base()->scheme_information(scheme);
                return component;
            }, std::move(scheme_args.args))...);

            std::apply([](auto&&... entities) {
                (..., entities->base()->scheme_created());
            }, std::move(entities));
        });

        return id;
    }

private:
    inline tasks get_scheduler()
    {
        thread_local tasks ts;
        return ts;
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
    std::vector<std::thread> _workers;
    boost::fibers::mutex _mutex;
    boost::fibers::condition_variable_any _cv;
    boost::fibers::barrier _threads_joined;

    uint64_t _current_id;
    std::vector<uint64_t> _free_ids;
};
