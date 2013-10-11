#include "./spawn.h"

#include "./detail/allocator.h"

#include <iostream>

namespace {

void DefaultSpawnHandler() {
	// DoNothing
}

void CoroEntryPoint(coro::Function& coroutine, coro::Context context) {
	coro::detail::Allocator allocator;
	context.handler_.SetAllocator(allocator);
	try {
		coroutine(context);
	} catch (const std::exception& e) {
		std::cerr << "exception in coroutine execution: " << e.what() << std::endl;
		throw;
	} catch (...) {
		std::cerr << "unknown exception in coruoutine execution" << std::endl;
		throw;
	}	
}

}

void coro::Spawn(boost::asio::io_service& ioService, coro::Function&& coroutine) {
	coro::Handler handler{ioService.wrap(DefaultSpawnHandler)};
	std::function<void(coro::Context)> coroEntryPoint{std::bind(CoroEntryPoint, std::forward<coro::Function>(coroutine), std::placeholders::_1)};
	boost::asio::spawn(handler, std::move(coroEntryPoint));
}
