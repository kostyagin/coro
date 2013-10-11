#pragma once

#include "executor.h"

namespace coro {
namespace function {

class SpawnExecutor final: public coro::function::Executor {

public:
	explicit SpawnExecutor(boost::asio::io_service& ioService);

private:
	// Executor
	virtual void Execute(coro::Function&& function) override final;	
};

}
}