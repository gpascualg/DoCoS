#pragma once

#include "common/types.hpp"
#include "containers/pooled_static_vector.hpp"

#include <unordered_map>


template <typename T>
class store
{
    template <typename D, typename B, uint16_t S> friend class dictionary;

public:
    static inline typename ticket<T>::ptr get(entity_id_t id)
    {
        if (auto it = _tickets.find(id); it != _tickets.end())
        {
            return it->second;
        }
        return nullptr;
    }

protected:
    static inline std::unordered_map<entity_id_t, typename ticket<T>::ptr> _tickets;
};

template <typename T, typename B, uint16_t InitialSize>
class dictionary : public pooled_static_vector<T, B, InitialSize, dictionary<T, B, InitialSize>>, public store<B>
{
    friend class pooled_static_vector<T, B, InitialSize, dictionary>;
    template <typename... D> friend class scheme;

public:
    dictionary();

    template <uint16_t OtherSize>
    B* move(entity_id_t id, dictionary<T, B, OtherSize>& to);

    template <uint16_t OtherSize>
    B* move(const typename ticket<B>::ptr ticket, dictionary<T, B, OtherSize>& to);

protected:
    void register_alloc(entity_id_t id, T* object);
    void register_free(entity_id_t id);
};


template <typename T, typename B, uint16_t InitialSize>
dictionary<T, B, InitialSize>::dictionary() :
    pooled_static_vector<T, B, InitialSize, dictionary<T, B, InitialSize>>()
{}


template <typename T, typename B, uint16_t InitialSize>
template <uint16_t OtherSize>
B* dictionary<T, B, InitialSize>::move(entity_id_t id, dictionary<T, B, OtherSize>& to)
{
    return move(store<B>::get(id), to);
}

template <typename T, typename B, uint16_t InitialSize>
template <uint16_t OtherSize>
B* dictionary<T, B, InitialSize>::move(const typename ticket<B>::ptr ticket, dictionary<T, B, OtherSize>& to)
{
    return pooled_static_vector<T, B, InitialSize, dictionary<T, B, InitialSize>>::move_impl(
        reinterpret_cast<T*>(ticket->get()), to);
}

template <typename T, typename B, uint16_t InitialSize>
void dictionary<T, B, InitialSize>::register_alloc(entity_id_t id, T* object)
{
    store<B>::_tickets.emplace(id, object->ticket());
}

template <typename T, typename B, uint16_t InitialSize>
void dictionary<T, B, InitialSize>::register_free(entity_id_t id)
{
    store<B>::_tickets.erase(id);
}
