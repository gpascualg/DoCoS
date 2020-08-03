#pragma once

#include "common/definitions.hpp"
#include <entity/entity.hpp>

#include <boost/asio.hpp>


using udp = boost::asio::ip::udp;

class client : public entity<client>
{
public:
    void construct(udp::endpoint endpoint);
    void update(const base_time& diff);
};
