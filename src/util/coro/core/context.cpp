#include "./context.h"

boost::asio::io_service& coro::GetIoService(const coro::Context& context) {
	return context.handler_.dispatcher_;
}