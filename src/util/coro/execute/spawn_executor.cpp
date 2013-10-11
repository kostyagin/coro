#include "./spawn_executor.h"

#include "src/util/coro/core/spawn.h"

coro::function::SpawnExecutor::SpawnExecutor(boost::asio::io_service& ioService)
	: coro::function::Executor{ioService}
{
}

void coro::function::SpawnExecutor::Execute(coro::Function&& function) {
	coro::Spawn(GetIoService(), std::forward<coro::Function>(function));
}
