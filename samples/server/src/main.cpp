#include "core/server.hpp"

#include <boost/fiber/mutex.hpp>
#include <boost/fiber/condition_variable.hpp>

#include <glm/gtx/string_cast.hpp>

#include <iostream>


int main()
{
    server server(7575, 3, 2);
    server.mainloop();
    server.stop();
    return 0;
}
