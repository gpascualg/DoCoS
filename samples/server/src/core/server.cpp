#include "core/server.hpp"


server::server(uint8_t num_workers) :
    executor(num_workers, false),
    _store(),
    _map_scheme(_map_scheme.make(_store)),
    _client_scheme(_client_scheme.make(_store)),
    _last_tick(std_clock_t::now()),
    _diff_mean(static_cast<float>(HeartBeat.count())),
    _stop(false)
{}

void server::mainloop()
{
    auto scheme = overlap(_store, _map_scheme, _client_scheme);
    auto updater = scheme.make_updater();

    get_or_create_client(udp::endpoint(), [](auto client) {
        return std::tuple(client);
    });

    while (!_stop)
    {
        auto now = std_clock_t::now();
        auto diff = elapsed(_last_tick, now);
        _diff_mean = 0.95f * _diff_mean + 0.05f * diff.count();

        executor::update(updater, diff);
        executor::sync(updater, diff);
        executor::execute_tasks();

        _last_tick = now;
        auto update_time = elapsed(now, std_clock_t::now());
        if (update_time < HeartBeat)
        {
            auto diff_mean = base_time(static_cast<uint64_t>(_diff_mean));
            auto sleep_time = HeartBeat - update_time - (diff_mean - HeartBeat);
            std::cout << "Diff / Sleep / Mean = " << diff.count() << "/" << sleep_time.count() << "/" << _diff_mean << std::endl;
            std::this_thread::sleep_for(sleep_time);
        }
    }
}

void server::stop()
{
    executor::stop();
}

client* server::get_client(const udp::endpoint& endpoint) const
{
    if (auto it = _clients.find(endpoint); it != _clients.end())
    {
        if (auto client = it->second.get())
        {
            return client->derived();
        }
    }
    
    return nullptr;
}
