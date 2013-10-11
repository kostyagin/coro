#pragma once

#include "./detail/parking.h"

#include "src/util/check/check.h"
#include "src/util/container/queue.h"
#include "src/util/container/stack.h"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

#include <functional>
#include <type_traits>

namespace coro {

template <bool fifo = true>
class SharedCondition: private boost::noncopyable {

public:
	explicit SharedCondition(boost::asio::io_service& ioService);
	~SharedCondition();

	boost::asio::io_service& GetIoService();

	void Wait(const coro::Context& context);

	size_t NotifyOne();
	size_t NotifyAll();

private:
	using StoredHandler = coro::detail::Parking::Handler;
	using Container = typename std::conditional<fifo, container::Queue<StoredHandler*>, container::Stack<StoredHandler*>>::type;

	boost::asio::io_service& ioService;
	Container handlers;
	size_t notified;

	void Store(StoredHandler& handler);
	void DoNotifyOne();
};

template <bool fifo>
inline SharedCondition<fifo>::SharedCondition(boost::asio::io_service& ioService)
	: ioService{ioService}
	, notified(0)
{
}

template <bool fifo>
inline SharedCondition<fifo>::~SharedCondition() {
	Check(handlers.Empty());
	Check(notified == 0);
}

template <bool fifo>
inline void SharedCondition<fifo>::Wait(const coro::Context& context) {
	coro::detail::Parking parking(context);
	Store(parking.GetHandler());
	parking.Wait();
}

template <bool fifo>
inline boost::asio::io_service& SharedCondition<fifo>::GetIoService() {
	return ioService;
}

template <bool fifo>
inline size_t SharedCondition<fifo>::NotifyOne() {
	if (handlers.Size() == notified) {
		return 0;
	}
	Check(handlers.Size() > notified);
	++notified;
	ioService.post(std::bind(&SharedCondition<fifo>::DoNotifyOne, this));
	return 1;
}

template <bool fifo>
inline size_t SharedCondition<fifo>::NotifyAll() {
	size_t result = 0;
	while (handlers.Size() != notified) {
		result += NotifyOne();
	}
	return result;
}

template <bool fifo>
inline void SharedCondition<fifo>::Store(StoredHandler& handler) {
	handlers.Push(&handler);
}

template <bool fifo>
inline void SharedCondition<fifo>::DoNotifyOne() {
	StoredHandler handler{std::move(*handlers.Top())};
	handlers.Pop();
	--notified;
	handler();
}

}
