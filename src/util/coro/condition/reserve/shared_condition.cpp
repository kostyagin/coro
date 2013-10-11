#include "./shared_condition.h"

#include "src/util/check/check.h"

#include <boost/chrono/duration.hpp>

#include <limits>

coro::reserve::SharedCondition::SharedCondition(boost::asio::io_service& ioService)
	: timer{ioService, std::chrono::hours(std::numeric_limits<std::chrono::hours::rep>::max())}
{
}

void coro::reserve::SharedCondition::Wait(const coro::Context& context)
{
	boost::system::error_code ec;
	coro::Context ecContext = context[ec];
	timer.async_wait(ecContext);
	CheckWithMessage(ec == boost::asio::error::operation_aborted, "coro::Condition timer expired");
}

size_t coro::reserve::SharedCondition::NotifyOne()
{
	return timer.cancel_one();
}

size_t coro::reserve::SharedCondition::NotifyAll()
{
	return timer.cancel();
}
