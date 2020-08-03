#pragma once

#include "common/types.hpp"
#include "containers/pool_item.hpp"
#include "containers/dictionary.hpp"


template <typename derived_t>
class entity : public pool_item<entity<derived_t>>
{
    template <typename D, typename E, uint16_t I, typename R> friend class pooled_static_vector;
    template <typename... types> friend class updater;
    friend class executor;

public:
    inline entity_id_t id();

    template <typename... Args>
    static inline constexpr bool has_update()
    {
        return ::has_update<derived_t, Args...>;
    }

    template <typename... Args>
    static inline constexpr bool has_sync()
    {
        return ::has_sync<derived_t, Args...>;
    }

    inline entity<derived_t>* base()
    {
        return this;
    }
    
    template <typename T>
    inline T* get()
    {
        if (auto ticket = store<entity<T>>::get(id()))
        {
            return ticket->template get<T>();
        }
        
        return nullptr;
    }

private:
    template <typename... Args>
    constexpr inline void update(Args&&... args);

    template <typename... Args>
    constexpr inline void sync(Args&&... args);

    template <typename... Args>
    constexpr inline void construct(entity_id_t id, Args&&... args);

    template <typename... Args>
    constexpr inline void destroy(Args&&... args);

    constexpr inline void scheme_created();

    template <template <typename...> typename S, typename... components>
    constexpr inline void scheme_information(const S<components...>& scheme);

private:
    entity_id_t _id;
};


template <typename derived_t>
inline entity_id_t entity<derived_t>::id()
{
    return _id;
}

template <typename derived_t>
template <typename... Args>
constexpr inline void entity<derived_t>::construct(entity_id_t id, Args&&... args)
{
    _id = id;

#if (__DEBUG__ || FORCE_ALL_CONSTRUCTORS) && !DISABLE_DEBUG_CONSTRUCTOR
    static_cast<derived_t&>(*this).construct(std::forward<Args>(args)...);
#else
    if constexpr (constructable<derived_t, Args...>)
    {
        static_cast<derived_t&>(*this).construct(std::forward<Args>(args)...);
    }
#endif
}

template <typename derived_t>
template <typename... Args>
constexpr inline void entity<derived_t>::destroy(Args&&... args)
{
    if constexpr (destroyable<derived_t, Args...>)
    {
        static_cast<derived_t&>(*this).destroy(std::forward<Args>(args)...);
    }
}

template <typename derived_t>
template <typename... Args>
constexpr inline void entity<derived_t>::update(Args&&... args)
{
    if constexpr (::has_update<derived_t, Args...>)
    {
        static_cast<derived_t&>(*this).update(std::forward<Args>(args)...);
    }
}

template <typename derived_t>
template <typename... Args>
constexpr inline void entity<derived_t>::sync(Args&&... args)
{
    if constexpr (::has_sync<derived_t, Args...>)
    {
        static_cast<derived_t&>(*this).sync(std::forward<Args>(args)...);
    }
}

template <typename derived_t>
constexpr inline void entity<derived_t>::scheme_created()
{
    if constexpr (has_scheme_created<derived_t>)
    {
        static_cast<derived_t&>(*this).scheme_created();
    }
}

template <typename derived_t>
template <template <typename...> typename S, typename... components>
constexpr inline void entity<derived_t>::scheme_information(const S<components...>& scheme)
{
    if constexpr (has_scheme_information<derived_t, S, components...>)
    {
        static_cast<derived_t&>(*this).scheme_information(scheme);
    }
}
