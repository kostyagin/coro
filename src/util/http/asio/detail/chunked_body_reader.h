#pragma once

#include "./chunked_body_compactor.h"

#include "src/util/coro/core/context.h"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace container {
class Buffer;
}

namespace http {
namespace asio {
namespace detail {

class ChunkedBodyReader final: private boost::noncopyable {

public:
	ChunkedBodyReader(boost::asio::ip::tcp::socket& socket, container::Buffer& buffer, size_t headersLength);

	void Read(const coro::Context& context, bool& connectionClose);

private:
	boost::asio::ip::tcp::socket& socket;
	container::Buffer& buffer;
	ChunkedBodyCompactor compactor;
	size_t readingFrom;
	size_t lastBytesTransferred;

	size_t CompletionCondition(const boost::system::error_code& error, size_t bytesTransferred);
};

}
}
}
