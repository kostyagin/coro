#pragma once

#include "./executor.h"

#include "src/util/container/queue.h"
#include "src/util/coro/condition/shared_condition.h"

namespace coro {
namespace function {

class PooledExecutor final: public coro::function::Executor {

public:
	explicit PooledExecutor(boost::asio::io_service& ioService);
	virtual ~PooledExecutor() override final;

private:
	coro::Function coroutineFunc;
	struct ControlInfo {
		size_t size;
		bool needStop;
	};
	ControlInfo* controlInfo;
	container::Queue<coro::Function> toExecute;
	coro::SharedCondition<false> condition;
	
	// coro::function::Executor
	virtual void Execute(coro::Function&& function) override final;

	void StartNewCoroutine();
	void CoroutineFunc(const coro::Context& context);
};

}	
}