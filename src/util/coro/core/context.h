#pragma once

#include "./handler.h"

namespace coro {

using Context = boost::asio::basic_yield_context<coro::Handler>;

boost::asio::io_service& GetIoService(const coro::Context& context);

}
