#include "./pooled_executor.h"

#include "src/util/check/check.h"
#include "src/util/coro/core/spawn.h"

#include <functional>

coro::function::PooledExecutor::PooledExecutor(boost::asio::io_service& ioService)
	: coro::function::Executor(ioService)
    , coroutineFunc(std::bind(&PooledExecutor::CoroutineFunc, this, std::placeholders::_1))
	, controlInfo(new ControlInfo{0, false})
	, condition(ioService)	
{
}

coro::function::PooledExecutor::~PooledExecutor() {
	Check(toExecute.Empty());
	controlInfo->needStop = true;
	condition.NotifyAll();
}

void coro::function::PooledExecutor::Execute(coro::Function&& function) {
	toExecute.Push(std::forward<coro::Function>(function));
	if (condition.NotifyOne() == 0) {
		StartNewCoroutine();
	}
}

void coro::function::PooledExecutor::StartNewCoroutine() {
	coro::Spawn(GetIoService(), std::bind(std::cref(coroutineFunc), std::placeholders::_1));
	++controlInfo->size;
}

void coro::function::PooledExecutor::CoroutineFunc(const coro::Context& context) {
	ControlInfo* controlInfo = this->controlInfo;
	Check(!controlInfo->needStop);
	do {
		while (!toExecute.Empty()) {
			coro::Function function{std::move(toExecute.Top())};
			toExecute.Pop();
			function(context);
			if (controlInfo->needStop) {
				break;
			}
		}
		condition.Wait(context);
	} while (!controlInfo->needStop);
	if (--controlInfo->size == 0) {
		delete controlInfo;
	}
}
