#pragma once

#include "src/util/coro/condition/condition.h"
#include "src/util/coro/core/context.h"
#include "src/util/coro/core/function.h"

#include <boost/noncopyable.hpp>

#include <chrono>

namespace coro {
namespace function {
class Executor;
}
}

namespace curl {
namespace multi_socket {
namespace detail {

class Timer final: public boost::noncopyable {

public:
	using Callback = std::function<void()>;

	Timer(coro::function::Executor& executor, Callback&& callback);
	void SetTimeout(long timeout); // В миллисекундах, > 0, предыдущий отменяется
	void Cancel(); // Отменить таймаут

private:
	const coro::Function coro;
	const Callback callback;
	coro::function::Executor& executor;
	bool hasTimeout;
	int generation;
	boost::asio::basic_waitable_timer<std::chrono::steady_clock> timer;
	bool coroStarted;

	void StartCoro();
	void Coro(const coro::Context& context);
};

}
}
}
