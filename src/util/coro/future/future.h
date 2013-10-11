#pragma once

#include "src/util/check/check.h"
#include "src/util/coro/condition/condition.h"
#include "src/util/coro/core/context.h"

#include <boost/asio/io_service.hpp>
#include <boost/noncopyable.hpp>

#include <type_traits>

namespace coro {

namespace details {

class FutureBase: private boost::noncopyable {
public:
	explicit FutureBase(boost::asio::io_service& ioService);

	void Wait(const coro::Context& context);
	void Get(const coro::Context& context);
	void Set();
	void SetException(std::exception_ptr exception);
	void Reset();
private:
	bool hasState;
	std::exception_ptr exception;
	coro::Condition condition;
};

}

template <class T>
class Future final {
public:
	explicit Future(boost::asio::io_service& ioService);

	void Wait(const coro::Context& context);
	T Get(const coro::Context& context);
	template <class... Args> void Set(Args&&... args);
	void SetException(std::exception_ptr exception);
	void Reset();

private:
	details::FutureBase base;
	typename std::aligned_storage<sizeof(T), std::alignment_of<T>::value>::type storage;
};

template <>
class Future<void> final: public details::FutureBase {
public:
	explicit Future(boost::asio::io_service& ioService);
};

template <class T>
inline Future<T>::Future(boost::asio::io_service& ioService)
	: base(ioService)
{
}

template <class T>
inline void Future<T>::Wait(const coro::Context& context) {
	base.Wait(context);
}

template <class T>
inline T Get(const coro::Context& context)
{
	base.Get(context);
	void* valuePlace = &storage;
	T& value = *static_cast<T*>(valuePlace);
	T result = std::move(value);
	value.~T();
	return result;
}

template <class T>
template <class... Args>
inline void Future<T>::Set(Args&&... args)
{
	void* valuePlace = &storage;
	new (valuePlace) T(std::forward<Args>(args)...);
	base.Set();
}

template <class T>
inline void Future<T>::SetException(std::exception_ptr exception)
{
	base.SetException();
}

}
