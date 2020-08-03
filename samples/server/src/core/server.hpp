#pragma once

#include "maps/map.hpp"

#include <entity/scheme.hpp>
#include <updater/executor.hpp>


constexpr uint32_t prereserved_size = 256;

class server : protected executor
{
    template <typename T, uint32_t S = prereserved_size> using dic = dictionary<T, entity<T>, S>;

public:
    server(uint8_t num_workers);

    void mainloop();
    void stop();

private:
    scheme_store<dic<map>> _store;
    decltype(scheme<map>::make(_store)) _map_scheme;
    
    bool _stop;
};
