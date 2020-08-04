#pragma once

#include "containers/concepts.hpp"
#include "containers/ticket.hpp"



template <typename T>
class pool_item
{
    template <typename D, typename E, uint16_t I, typename R> friend class pooled_static_vector;

public:
    pool_item();
    pool_item(const pool_item&) = delete;
    pool_item(pool_item&&) = default;
    pool_item& operator=(const pool_item&) = delete;
    pool_item& operator=(pool_item&&) = default;

    inline typename ticket<T>::ptr ticket() { return _ticket; }

private:
   typename ::ticket<T>::ptr _ticket;
};

template <typename T>
pool_item<T>::pool_item() :
    _ticket(nullptr)
{}
