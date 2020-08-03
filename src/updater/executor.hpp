#pragma once

#include "containers/thread_local_tasks.hpp"
#include "ids/generator.hpp"
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
    executor(uint8_t num_workers, bool suspend) :
        _threads_joined(num_workers)
    {
        // What if there is more than one executor?
        instance = this;

        for (uint8_t thread_id = 1; thread_id < num_workers; ++thread_id)
        {
            _workers.push_back(std::thread(
                [this, num_workers, suspend] {
                    // Set thread algo
                    public_scheduling_algorithm<exclusive_work_stealing<0>>(num_workers, suspend);

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
        public_scheduling_algorithm<exclusive_work_stealing<0>>(num_workers, suspend);
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
    void update(U& updater, Args&&... args)
    {
        boost::fibers::fiber([&updater, ...args{ std::forward<Args>(args) }]() mutable {
            updater.update(std::forward<Args>(args)...);
        }).join();
    }

    template <typename U, typename... Args>
    void sync(U& updater, Args&&... args)
    {
        updater.sync(std::forward<Args>(args)...);
    }

    template <template <typename...> typename S, typename... A, typename... vecs>
    constexpr uint64_t create(S<vecs...>& scheme, A&&... scheme_args)
    {
        return create_with_callback(scheme, [](auto&&... e) { return std::tuple(e...); }, std::forward<A>(scheme_args)...);
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
        uint64_t id = id_generator<S<vecs...>>().next();
        return create_with_callback(id, scheme, std::move(callback), std::forward<A>(scheme_args)...);
    }

    template <template <typename...> typename S, typename C, typename... A, typename... vecs>
    constexpr uint64_t create_with_callback(uint64_t id, S<vecs...>& scheme, C&& callback, A&&... scheme_args)
    {
        static_assert(sizeof...(vecs) == sizeof...(scheme_args), "Incomplete scheme creation");

        schedule([id, callback{ std::move(callback) }, ...scheme_args{ std::move(scheme_args) }, &scheme]{
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

protected:
    inline tasks& get_scheduler()
    {
        thread_local tasks ts;
        return ts;
    }

    template <typename T>
    inline generator<T>& id_generator()
    {
        thread_local generator<T> gen;
        return gen;
    }

private:
    std::vector<std::thread> _workers;
    boost::fibers::mutex _mutex;
    boost::fibers::condition_variable_any _cv;
    boost::fibers::barrier _threads_joined;
};
