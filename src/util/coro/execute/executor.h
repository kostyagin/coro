#pragma once

#include "src/util/coro/core/function.h"

#include <boost/noncopyable.hpp>

namespace coro {
namespace function {

class Executor: private boost::noncopyable {

public:
	explicit Executor(boost::asio::io_service& ioService);
	virtual ~Executor();
	boost::asio::io_service& GetIoService();
	virtual void Execute(coro::Function&& function) = 0;

private:
	boost::asio::io_service& ioService;
};

}
}