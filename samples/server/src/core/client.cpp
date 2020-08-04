#include "core/client.hpp"

#include <iostream>


void client::construct(const udp::endpoint& endpoint)
{
    _endpoint = endpoint;
}

void client::update(const base_time& diff)
{}

void client::push_data(udp_buffer* buffer, uint16_t size)
{
    
}