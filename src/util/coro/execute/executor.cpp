#include "./executor.h"

coro::function::Executor::Executor(boost::asio::io_service& ioService)
	: ioService(ioService)
{
}

coro::function::Executor::~Executor() {
}

boost::asio::io_service& coro::function::Executor::GetIoService() {
	return ioService;
}