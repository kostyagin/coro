#pragma once

#include "src/util/coro/core/context.h"

#include <boost/asio/basic_waitable_timer.hpp>

#include <chrono>

namespace coro {
namespace reserve {

class SharedCondition final {

public:
	explicit SharedCondition(boost::asio::io_service& ioService);

	void Wait(const coro::Context& context);
	size_t NotifyOne();
	size_t NotifyAll();

private:
	boost::asio::basic_waitable_timer<std::chrono::steady_clock> timer;
};

}
}