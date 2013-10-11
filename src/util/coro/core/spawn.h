#pragma once

#include "./function.h"

#include <functional>

namespace coro {

void Spawn(boost::asio::io_service& ioService, coro::Function&& coroutine);

}