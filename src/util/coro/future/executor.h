#pragma once

#include "./future.h"

#include "src/util/container/queue.h"
#include "src/util/coro/core/context.h"
#include "src/util/coro/execute/executor.h"

#include <boost/noncopyable.hpp>

#include <functional>

namespace coro {
namespace future {

class Executor final: private boost::noncopyable {
public:
	explicit Executor(coro::function::Executor& coroExecutor);

	boost::asio::io_service& GetIoService();

	// Чтобы избежать аллокаций, функция должна быть не выше второго ранга,
	// т.к. ещё один будет добавлен для убирания типа и сохранения
	template <class T>
	void Execute(std::function<T(const coro::Context&)>&& function, coro::Future<T>& future);
	// Тут функция может быть не выше третьего ранга чтобы избежать аллокаций
	void Execute(coro::Function&& function, coro::Future<void>& future);

private:
	coro::function::Executor& coroExecutor;
	using StoredTypedFunction = std::function<void(const coro::Context&, void*)>;
	using StoredTypedData = std::pair<StoredTypedFunction, void*>;
	using StoredVoidData = std::pair<coro::Function, void*>;
	coro::Function executeTypedImpl;
	coro::Function executeVoidImpl;
	container::Queue<StoredTypedData> typedQueue;
	container::Queue<StoredVoidData> voidQueue;

	template <class T>
	static void ToStoredTyped(std::function<T(const coro::Context&)>& function, const coro::Context& context, void* futurePointer);
	void ExecuteTyped(StoredTypedFunction&& storedTypedFunction, void* storedFuture);
	void ExecuteVoid(coro::Function&& function, void* storedFuture);
	void ExecuteTypedImpl(const coro::Context& context);
	void ExecuteVoidImpl(const coro::Context& context);
};

template <class T>
inline void Executor::Execute(std::function<T(const coro::Context&)>&& function, coro::Future<T>& future) {
	future.Reset();
	ExecuteTyped(std::bind(ToStoredTyped<T>, function, std::placeholders::_1, std::placeholders::_2), &future);
}

template <class T>
static void Executor::ToStoredTyped(std::function<T(const coro::Context&)>& function, const coro::Context& context, void* futurePointer) {
	coro::Future<T>& future = *staitc_cast<coro::Future<T>*>(futurePointer);
	std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type place;
	try {
		new (&place) T(function(context));
	} catch (...) {
		future.SetException(std::current_exception());
		return;
	}
	void* valuePlace = &place;
	T& value = *static_cast<T*>(valuePlace);
	future.SetValue(std::move(value));
	value::~T();	
}

}
}
