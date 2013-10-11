#include "./timer.h"

#include "src/util/check/check.h"
#include "src/util/coro/execute/executor.h"

curl::multi_socket::detail::Timer::Timer(coro::function::Executor& executor, Callback&& callback)
	: coro(std::bind(&Timer::Coro, this, std::placeholders::_1))
	, callback(std::forward<Callback>(callback))
	, executor(executor)
	, hasTimeout(false)
	, generation(0)
	, timer(executor.GetIoService())
	, coroStarted(false)
{
}

void curl::multi_socket::detail::Timer::SetTimeout(long timeout) {
	Cancel();
	timer.expires_from_now(std::chrono::milliseconds(timeout));
	hasTimeout = true;
	++generation;
	if (!coroStarted) {
		StartCoro();
	}
}

void curl::multi_socket::detail::Timer::Cancel() {
	const size_t numberOfCanceled = timer.cancel();
	Check(numberOfCanceled <= 1);
	hasTimeout = false;
}

void curl::multi_socket::detail::Timer::StartCoro() {
	Check(!coroStarted);
	coro::Function coro(this->coro);
	coroStarted = true;
	executor.Execute(std::move(coro));
}

void curl::multi_socket::detail::Timer::Coro(const coro::Context& context) {
	Check(coroStarted);
	while (hasTimeout) {
		const int waitForGeneration = generation;
		boost::system::error_code ec;
		timer.async_wait(context[ec]);
		if (!ec) {
			if (hasTimeout && waitForGeneration == generation) {
				callback();
			}
		} else if (ec != boost::asio::error::operation_aborted) {
			throw boost::system::system_error(ec);
		}
	}
	coroStarted = false;
}
