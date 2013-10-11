#pragma once

#include "src/util/coro/core/context.h"

#include <boost/noncopyable.hpp>

namespace coro {
namespace detail {

class Parking: private boost::noncopyable {

public:
	using Handler = boost::asio::detail::coro_handler<coro::Handler, void>;

	explicit Parking(const coro::Context& context);
	
	Handler& GetHandler();
	void Wait();

private:
	boost::asio::detail::async_result_init<const coro::Context&, void ()> asyncResultInit;
};

inline Parking::Parking(const coro::Context& context)
	: asyncResultInit(context)
{
}

inline Parking::Handler& Parking::GetHandler() {
	return asyncResultInit.handler;
}

inline void Parking::Wait() {
	asyncResultInit.result.get();
}

}
}
