#pragma once

#include "src/util/container/slice/slice.h"
#include "src/util/coro/core/context.h"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace container {
class Buffer;
};

namespace http {
namespace asio {
namespace detail {

class BodyReader final: public boost::noncopyable {

public:
	BodyReader(boost::asio::ip::tcp::socket& socket, container::Buffer& buffer, size_t headersLength);
	
	container::Slice Read(const coro::Context& context, bool isChunked, size_t contentLength, bool canHaveBody, bool& connectionClose);
	
private:
	boost::asio::ip::tcp::socket& socket;
	container::Buffer& buffer;
	const size_t headersLength;

	void ReadChunked(const coro::Context& context, bool& connectionClose);
	void ReadSolid(const coro::Context& context, size_t contentLength, bool& connectionClose);
	void ReadEmpty(const coro::Context& context);
	void ReadUntilEof(const coro::Context& context);
}; 

}
}
}
