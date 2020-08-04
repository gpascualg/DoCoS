#pragma once

#include "common/definitions.hpp"
#include <entity/entity.hpp>

#include <boost/asio.hpp>


using udp = boost::asio::ip::udp;

class client : public entity<client>
{
public:
    void construct(const udp::endpoint& endpoint);
    void update(const base_time& diff);

    void push_data(udp_buffer* buffer, uint16_t size);

private:
    udp::endpoint _endpoint;
};
