#pragma once

#include "containers/pool_item.hpp"

#include <array>
#include <inttypes.h>
#include <memory>
#include <queue>
#include <vector>

#include <boost/pool/pool.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/join.hpp>


template <poolable T, typename B, uint16_t InitialSize, typename Track>
class pooled_static_vector;



template <poolable T, typename B, uint16_t InitialSize, typename Track=void>
class pooled_static_vector
{
    template <typename... D> friend class scheme;

public:
    using base_t = B;
    using derived_t = T;

public:
    pooled_static_vector();
    ~pooled_static_vector();

    template <typename... Args>
    void clear(Args&&... args);

    template <typename... Args>
    T* alloc(Args&&... args);

    template <typename... Args>
    void free(T* object, Args&&... args);

    inline bool is_static(T* object) const;
    inline bool is_static_full() const;
    inline bool empty() const;
    inline size_t size() const;
    
    inline auto range()
    {
        return boost::join(
            boost::adaptors::transform(
                boost::adaptors::slice(_objects, 0, static_cast<std::size_t>(_current - &_objects[0])),
                [](T& obj) { return &obj; }), 
            _extra);
    }

protected:
    template <uint16_t OtherSize, typename R>
    T* move_impl(T* object, pooled_static_vector<T, B, OtherSize, R>& to);
    T* move_impl(T* object);

private:
    void free_impl(T* object);

private:
    T* _current;
    T* _end;
    std::array<T, InitialSize> _objects;
    std::vector<T*> _extra;
    boost::pool<> _pool;
};


template <poolable T, typename B, uint16_t InitialSize, typename Track>
pooled_static_vector<T, B, InitialSize, Track>::pooled_static_vector() :
    _objects(),
    _pool(sizeof(T))
{
    _current = &_objects[0];
    _end = &_objects[0] + InitialSize;
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
pooled_static_vector<T, B, InitialSize, Track>::~pooled_static_vector()
{
    clear();
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
template <typename... Args>
void pooled_static_vector<T, B, InitialSize, Track>::clear(Args&&... args)
{
    // TODO(gpascualg): Destroying entities here might block too much once we have DB
    // Probably we could temporary move ourselves to the DB thread and
    // progressively add them to said DB
    for (auto object : range())
    {
        static_cast<B&>(*object).destroy(std::forward<Args>(args)...);
        object->ticket()->invalidate();
    }

    // Point static to start again
    _current = &_objects[0];

    // Clear dynamic
    _pool.purge_memory();
    _extra.clear();
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
template <typename... Args>
T* pooled_static_vector<T, B, InitialSize, Track>::alloc(Args&&... args)
{
    // Get last item available
    T* object = _current;
    if (is_static_full())
    {
        void* ptr = _pool.malloc();
        object = new (ptr) T();
        _extra.push_back(object);
    }
    else
    {
        ++_current;
    }   
    
    // Get ticket to this position
    static_cast<B&>(*object).construct(std::forward<Args>(args)...);

    // Track it if necessary
    if constexpr (!std::is_same_v<Track, void>)
    {
        static_cast<Track&>(*this).register_alloc(std::get<0>(std::forward_as_tuple(args...)), object);
    }

    return object;
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
template <typename... Args>
void pooled_static_vector<T, B, InitialSize, Track>::free(T* object, Args&&... args)
{
    // Untrack it if necessary
    if constexpr (!std::is_same_v<Track, void>)
    {
        // TODO(gpascualg): Is there always id()? Enforce it?
        static_cast<Track&>(*this).register_free(object->id());
    }

    // Destroy obect
    static_cast<B&>(*object).destroy(std::forward<Args>(args)...);
    object->_ticket->invalidate();
    object->_ticket.reset();

    // Free memory
    free_impl(object);
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
inline bool pooled_static_vector<T, B, InitialSize, Track>::is_static(T* object) const
{
    return (object < _end) && (object >= &_objects[0]);
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
inline bool pooled_static_vector<T, B, InitialSize, Track>::is_static_full() const
{
    return _current == _end;
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
inline bool pooled_static_vector<T, B, InitialSize, Track>::empty() const
{
    return _current == &_objects[0];
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
inline size_t pooled_static_vector<T, B, InitialSize, Track>::size() const
{
    return _current - &_objects[0] + _extra.size();
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
template <uint16_t OtherSize, typename R>
T* pooled_static_vector<T, B, InitialSize, Track>::move_impl(T* object, pooled_static_vector<T, B, OtherSize, R>& to)
{
    // Write in destination pool and free memory
    T* new_object = to.move_impl(object);
    free_impl(object);
    return new_object;
}

template <poolable T, typename B, uint16_t InitialSize, typename Track>
T* pooled_static_vector<T, B, InitialSize, Track>::move_impl(T* object)
{
    // Insert in new location
    T* new_object = _current;
    if (is_static_full())
    {
        void* ptr = _pool.malloc();
        new_object = new (ptr) T(std::move(*object));
        _extra.push_back(new_object);
    }
    else
    {
        *new_object = std::move(*object);
        ++_current;
    }   
    
    // Update ticket
    new_object->_ticket->_ptr = new_object;
    return new_object;
}


template <poolable T, typename B, uint16_t InitialSize, typename Track>
void pooled_static_vector<T, B, InitialSize, Track>::free_impl(T* object)
{
    // Remove from current pool
    if (is_static(object))
    {
        T* replacement = nullptr;

        if (!_extra.empty())
        {
            replacement = _extra.back();
            _extra.pop_back();
            _pool.free(replacement);
        }
        else 
        {
            --_current;
            if (_current != object)
            {
                replacement = _current;
            }
        }

        if (replacement)
        {
            *object = std::move(*replacement);
            object->_ticket->_ptr = object;
        }
    }
    else
    {
        // There should be at least one element?
        // assert_true(_pool.is_from(object), "Wrong pool!");
        // assert_true(!_extra.empty(), "Freeing from an empty pool?");

        // If there is only this one extra object
        if (_extra.size() == 1)
        {
            _pool.free(object);
            _extra.clear();
        }
        else
        {
            // Do an optimized erase by moving last to object
            T* last = _extra.back();
            _extra.pop_back();
            if (last == object)
            {
                _pool.free(object);
            }
            else
            {
                *object = std::move(*last);
                object->_ticket->_ptr = object;
                _pool.free(last);
            }
        }
    }
}
