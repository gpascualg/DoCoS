#pragma once

#include <cstddef>

#include <boost/fiber/mutex.hpp>
#include <boost/pool/pool.hpp>


template <typename T>
class plain_pool
{
public:
    plain_pool(std::size_t size);

    template <typename... Args>
    T* get(Args&&... args);

    void free(T* object);

private:
    boost::pool<> _pool;
    boost::fibers::mutex _mutex;
};

template <typename T>
class singleton_pool : public plain_pool<T>
{
public:
    static inline singleton_pool* instance = nullptr;
    static inline void make(std::size_t size);

private:
    using plain_pool<T>::plain_pool;
};


template <typename T>
plain_pool<T>::plain_pool(std::size_t size) :
    _pool(size)
{}

template <typename T>
template <typename... Args>
T* plain_pool<T>::get(Args&&... args)
{
    _mutex.lock();
    void* ptr = _pool.malloc();
    _mutex.unlock();

    return new (ptr) T(std::forward<Args>(args)...);
}

template <typename T>
void plain_pool<T>::free(T* object)
{
    std::destroy_at(object);

    std::lock_guard<boost::fibers::mutex> lock{ _mutex };
    _pool.free(object);
}

template <typename T>
inline void singleton_pool<T>::make(std::size_t size)
{
    instance = new singleton_pool<T>(size);
}
