#pragma once

#include <inttypes.h>
#include <tuple>



template<typename T, typename... Args>
concept base_constructable = requires { (void (T::*)(Args...)) &T::construct; };

template<typename T, typename... Args>
concept base_destroyable = requires { (void (T::*)(Args...)) &T::destroy; };

template<typename T>
concept poolable = base_constructable<T, uint64_t> && base_destroyable<T>;

template <typename T, typename... Args>
concept constructable = requires(T* t, Args&&... args) {
    { t->construct(std::forward<Args  >(args)...) };
};

template <typename T, typename... Args>
concept destroyable = requires(T* t, Args&&... args) {
    { t->destroy(std::forward<Args>(args)...) };
};

template <typename T, typename... Args>
concept has_update = requires(T* t, Args&&... args) {
    { t->update(std::forward<Args>(args)...) };
};

template <typename T, typename... Args>
concept has_sync = requires(T* t, Args&&... args) {
    { t->sync(std::forward<Args>(args)...) };
};

template <typename T, template <typename...> typename S, typename... components>
concept has_scheme_information = requires(T* t, const S<components...>& s) {
    { t->scheme_information(s) };
};

template <typename T>
concept has_scheme_created = requires(T* t) {
    { t->scheme_created() };
};
