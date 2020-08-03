#pragma once

#include "common/definitions.hpp"
#include "core/client.hpp"
#include "maps/map.hpp"

#include <entity/scheme.hpp>
#include <updater/executor.hpp>

#include <unordered_map>
#include <chrono>


namespace std
{
    template <>
    struct hash<udp::endpoint>
    {
        size_t operator()(udp::endpoint const& v) const {
            uint64_t seed = 0;
            boost::hash_combine(seed, v.address().to_v4().to_uint());
            boost::hash_combine(seed, v.port());
            return seed;
        }
    };
}


constexpr uint32_t prereserved_size = 256;

class server : protected executor
{
    template <typename T, uint32_t S = prereserved_size> using dic = dictionary<T, entity<T>, S>;

public:
    server(uint8_t num_workers);

    void mainloop();
    void stop();

    client* get_client(const udp::endpoint& endpoint) const;
    template <typename C>
    void get_or_create_client(const udp::endpoint& endpoint, C&& callback);

private:
    scheme_store<dic<map>, dic<client>> _store;
    decltype(scheme<map>::make(_store)) _map_scheme;
    decltype(scheme<client>::make(_store)) _client_scheme;

    std::unordered_map<udp::endpoint, ticket<entity<client>>> _clients;

    std_clock_t::time_point _last_tick;
    float _diff_mean;

    bool _stop;
};


template <typename C>
void server::get_or_create_client(const udp::endpoint& endpoint, C&& callback)
{
    if (auto client = get_client(endpoint))
    {
        executor::schedule([client, callback{ std::move(callback) }](){
            callback(client);
        });

        return;
    }

    executor::create_with_callback(_client_scheme, std::move(callback),
        _client_scheme.args<client>(endpoint));
} 
