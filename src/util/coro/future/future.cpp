#include "./future.h"

coro::details::FutureBase::FutureBase(boost::asio::io_service& ioService)
	: hasState(false)
	, condition(ioService)
{
}

void coro::details::FutureBase::Wait(const coro::Context& context)
{
	if (hasState) {
		return;
	}
	condition.Wait(context);
	Check(hasState);	
}

void coro::details::FutureBase::Get(const coro::Context& context)
{
	Wait(context);
	Check(hasState);
	hasState = false;
	if (!exception) {
		return;
	}
	std::exception_ptr e = std::move(exception);
	exception = nullptr;
	std::rethrow_exception(e);
}

void coro::details::FutureBase::Set()
{
	Check(!hasState);
	hasState = true;
	condition.NotifyAll();
}

void coro::details::FutureBase::SetException(std::exception_ptr exception)
{
	Check(!hasState);
	Check(exception);
	Check(!this->exception);
	this->exception = exception;
	hasState = true;
	condition.NotifyAll();
}

void coro::details::FutureBase::Reset()
{
	exception = nullptr;
	hasState = false;	
}

coro::Future<void>::Future(boost::asio::io_service& ioService)
	: coro::details::FutureBase(ioService)
{
}