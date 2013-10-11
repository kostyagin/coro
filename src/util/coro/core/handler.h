#pragma once

#include "./detail/allocator.h"

#include <boost/asio/spawn.hpp>

namespace coro {

using DispatchHadnler = void(*)();
using BaseAsioHandler = boost::asio::detail::wrapped_handler<boost::asio::io_service&, coro::DispatchHadnler>;

class Handler: public BaseAsioHandler {

public:
	Handler(boost::asio::io_service& dispatcher, coro::DispatchHadnler& handler);
	Handler(const BaseAsioHandler& baseAsioHandler);

	void SetAllocator(coro::detail::Allocator& allocator);
	coro::detail::Allocator& GetAllocator();

private:
	Handler& operator=(const Handler&) = delete;
	coro::detail::Allocator* allocator;
};

inline Handler::Handler(boost::asio::io_service& dispatcher, coro::DispatchHadnler& handler)
	: BaseAsioHandler{dispatcher, handler}
	, allocator(nullptr)
{
}

inline Handler::Handler(const BaseAsioHandler& baseAsioHandler)
	: BaseAsioHandler(baseAsioHandler)
	, allocator(nullptr)
{
}

inline void Handler::SetAllocator(coro::detail::Allocator& allocator) {
	Check(!this->allocator);
	this->allocator = &allocator;
}

inline coro::detail::Allocator& Handler::GetAllocator() {
	Check(allocator);
	return *allocator;
}

// asio_handler_is_continuation(coro::Context*) не используем, т.к., судя по коду asio, оно знает, что тред один и тогда поступает так же, как в случае is_continuation

inline void* asio_handler_allocate(std::size_t size, coro::Handler* handler) {
	Check(handler);
	coro::detail::Allocator& allocator = handler->GetAllocator();
	void* const result = allocator.Allocate(size);
	return result ? result : boost::asio::asio_handler_allocate(size, handler);
}

inline void asio_handler_deallocate(void* pointer, std::size_t size, coro::Handler* handler) {
	Check(handler);
	coro::detail::Allocator& allocator = handler->GetAllocator();
	if (!allocator.Deallocate(pointer, size)) {
		boost::asio::asio_handler_deallocate(pointer, size, handler);
	}
}

}
