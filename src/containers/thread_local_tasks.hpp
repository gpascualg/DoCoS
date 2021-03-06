#pragma once

#include <boost/fiber/mutex.hpp>
#include <boost/fiber/fiber.hpp>

#include <vector>

template <typename T>
class thread_local_storage
{
public:
    using container_t = std::vector<T*>;

public:
    static inline void store(T* container)
    {
        _mutex.lock();
        _containers.push_back(container);
        _mutex.unlock();
    }

    static inline container_t& get()
    {
        return _containers;
    }

private:
    static inline boost::fibers::mutex _mutex;
    static inline container_t _containers;
};


class tasks
{
public:
    using task_t = std::function<void()>;
    using container_t = std::vector<task_t>;

public:
    tasks();
    void schedule(task_t&& task);
    void execute();

protected:
    void execute(container_t* buffer);

protected:
    container_t* _current_buffer;
    container_t _tasks_buffer1;
    container_t _tasks_buffer2;
};


class async_tasks : public tasks
{
public:
    using tasks::tasks;

    void schedule(task_t&& task);
    void execute();

private:
    boost::fibers::mutex _mutex;
};
