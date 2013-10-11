#pragma once

#include "./detail/parking.h"

#include "src/util/check/check.h"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>

#include <functional>

namespace coro {

class Condition : private boost::noncopyable {

public:
	explicit Condition(boost::asio::io_service& ioService);
	~Condition();

	boost::asio::io_service& GetIoService();
	
	void Wait(const coro::Context& context);

	size_t NotifyOne();
	size_t NotifyAll();

private:
	boost::asio::io_service& ioService;
	coro::detail::Parking::Handler* handler;
	bool notified;

	void DoNotifyOne();
};

inline Condition::Condition(boost::asio::io_service& ioService)
	: ioService{ioService}
	, handler{nullptr}
	, notified{false}
{
}

inline Condition::~Condition() {
	Check(!handler);
	Check(!notified);
}

inline boost::asio::io_service& Condition::GetIoService() {
	return ioService;
}

inline void Condition::Wait(const coro::Context& context) {
	coro::detail::Parking parking(context);
	handler = &parking.GetHandler();
	parking.Wait();
}

inline size_t Condition::NotifyOne() {
	if (!handler || notified) {
		return 0;
	}
	notified = true;
	ioService.post(std::bind(&Condition::DoNotifyOne, this));
	return 1;
}

inline size_t Condition::NotifyAll() {
	return NotifyOne();
}

inline void Condition::DoNotifyOne() {
	Check(notified && this->handler);
	coro::detail::Parking::Handler handler{std::move(*this->handler)};
	this->handler = nullptr;
	notified = false;
	handler();
}

}
