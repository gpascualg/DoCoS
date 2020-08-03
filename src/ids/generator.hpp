#pragma once

#include <atomic>
#include <inttypes.h>
#include <mutex>
#include <vector>


template <typename T>
class generator;

template <typename T>
class generators_collection
{
public:
    using container_t = std::vector<generator<T>*>;

public:
    static inline void store(generator<T>* ts)
    {
        _mutex.lock();
        _containers.push_back(ts);
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


template <typename T>
class generator
{
public:
    generator();

    uint64_t peek();
    uint64_t next();
    void free(uint64_t id);

    inline generator<T>& resize(uint64_t initial, uint64_t max);
    inline generator<T>& enable();

private:
    bool _disabled;

    // Current next id
    uint64_t _max;
    uint64_t _current;

    // Freed ids
    std::vector<uint64_t> _free;
};



template <typename T>
generator<T>::generator() :
    _disabled(true),
    _current(0),
    _max(0)
{
    generators_collection<T>::store(this);
}

template <typename T>
uint64_t generator<T>::peek()
{
    return _current - _free.size();
}

template <typename T>
uint64_t generator<T>::next()
{
    //assert_true(!_disabled, "Taking ID from disabled generator");

    // If there are free IDs, use those
    if (!_free.empty())
    {
        auto id = _free.back();
        _free.pop_back();
        return id;
    }

    // Take the next one
    auto id = _current++;
    //assert_true(id < _max, "ID overflow");
    return id;
}

template <typename T>
void generator<T>::free(uint64_t id)
{
    _free.push_back(id);
}

template <typename T>
inline generator<T>& generator<T>::resize(uint64_t initial, uint64_t max)
{
    _max = max;
    _current = initial;
    return *this;
}

template <typename T>
inline generator<T>& generator<T>::enable()
{
    _disabled = false;
    return *this;
}
