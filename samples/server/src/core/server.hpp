#pragma once

#include "common/definitions.hpp"
#include "core/client.hpp"
#include "maps/map.hpp"

#include <entity/scheme.hpp>
#include <updater/executor.hpp>

#include <boost/asio.hpp>

#include <unordered_map>
#include <unordered_set>
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
    static inline server* instance = nullptr;

public:
    server(uint16_t port, uint8_t num_server_workers, uint8_t num_network_workers);

    void mainloop();
    void stop();

    client* get_client(const udp::endpoint& endpoint) const;
    template <typename C>
    bool get_or_create_client(const udp::endpoint& endpoint, C&& callback);

private:
    void spawn_network_threads(uint8_t count);
    void handle_connections();

private:
    // DoCo schemes
    scheme_store<dic<map>, dic<client>> _store;
    decltype(scheme<map>::make(_store)) _map_scheme;
    decltype(scheme<client>::make(_store)) _client_scheme;

    // Clients
    std::unordered_map<udp::endpoint, ticket<entity<client>>> _clients;
    std::unordered_set<udp::endpoint> _blacklist;

    // Constant timestep
    std_clock_t::time_point _last_tick;
    float _diff_mean;

    // Networking
    std::vector<std::thread> _network_threads;
    boost::asio::io_context _context;
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> _work;
    udp::socket _socket;

    bool _stop;
};


template <typename C>
bool server::get_or_create_client(const udp::endpoint& endpoint, C&& callback)
{
    if (auto it = _blacklist.find(endpoint); it != _blacklist.end())
    {
        return false;
    }

    if (auto client = get_client(endpoint))
    {
        executor::schedule([client, callback{ std::move(callback) }](){
            callback(client);
        });

        return true;
    }

    executor::create_with_callback(_client_scheme, std::move(callback),
        _client_scheme.args<client>(endpoint));

    return true;
} 
