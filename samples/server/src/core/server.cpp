#include "core/server.hpp"


server::server(uint8_t num_workers) :
    executor(num_workers),
    _store(),
    _map_scheme(_map_scheme.make(_store)),
    _stop(false)
{}

void server::mainloop()
{
    auto map_updater = _map_scheme.make_updater();

    while (!_stop)
    {
        executor::update(map_updater);
        executor::sync(map_updater);
        executor::execute_tasks();
    }
}

void server::stop()
{
    executor::stop();
}
