#include "./executor.h"

coro::future::Executor::Executor(coro::function::Executor& coroExecutor)
	: coroExecutor{coroExecutor}
	, executeTypedImpl{std::bind(&Executor::ExecuteTypedImpl, this, std::placeholders::_1)}
	, executeVoidImpl{std::bind(&Executor::ExecuteVoidImpl, this, std::placeholders::_1)}
{
}

boost::asio::io_service& coro::future::Executor::GetIoService()
{
	return coroExecutor.GetIoService();
}

void coro::future::Executor::Execute(coro::Function&& function, coro::Future<void>& future) {
	future.Reset();
	ExecuteVoid(std::forward<coro::Function>(function), &future);
}

void coro::future::Executor::ExecuteTyped(StoredTypedFunction&& storedTypedFunction, void* storedFuture) {
	StoredTypedData storedTypedData{std::make_pair(std::forward<StoredTypedFunction>(storedTypedFunction), storedFuture)};
	typedQueue.Push(std::move(storedTypedData));
	coroExecutor.Execute(std::bind(std::cref(executeTypedImpl), std::placeholders::_1));
}

void coro::future::Executor::ExecuteVoid(coro::Function&& function, void* storedFuture) {
	StoredVoidData storedVoidData{std::make_pair(std::forward<coro::Function>(function), storedFuture)};
	voidQueue.Push(std::move(storedVoidData));
	coroExecutor.Execute(std::bind(std::cref(executeVoidImpl), std::placeholders::_1));
}

void coro::future::Executor::ExecuteTypedImpl(const coro::Context& context) {
	Check(!typedQueue.Empty());
	StoredTypedData storedTypedData{std::move(typedQueue.Top())};
	typedQueue.Pop();
	storedTypedData.first(context, storedTypedData.second);
}

void coro::future::Executor::ExecuteVoidImpl(const coro::Context& context) {
	Check(!voidQueue.Empty());
	StoredVoidData storedVoidData{std::move(voidQueue.Top())};
	voidQueue.Pop();
	coro::Function& function = storedVoidData.first;
	coro::Future<void>& future = *static_cast<coro::Future<void>*>(storedVoidData.second);
	try {
		function(context);
	} catch (...) {
		future.SetException(std::current_exception());
		return;
	}
	future.Set();
}
