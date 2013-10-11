#pragma once

#include "src/util/coro/core/context.h"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace container {
class Buffer;
}

namespace http {
namespace asio {
namespace detail {

class HeadersProcessor;

class HeadersReader final: private boost::noncopyable {

public:
	HeadersReader(boost::asio::ip::tcp::socket& socket, container::Buffer& buffer, HeadersProcessor& processor);

	// Возвращает длину заголовочной части в байтах, оставшаяся часть buffer содержит первый кусок body
	size_t Read(const coro::Context& context);

private:
	boost::asio::ip::tcp::socket& socket;
	container::Buffer& buffer;
	HeadersProcessor& processor;
	size_t readed;
	char* nextLineBegin;
	size_t lastBytesTransferred;
	bool headersReaded;

	size_t CompletionCondition(const boost::system::error_code& error, size_t bytesTransferred);
};

}
}
}
