#pragma once

#include "src/util/coro/core/context.h"

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace container {
class Buffer;
}

namespace http {
struct Response;
}

namespace http {
namespace asio {
namespace detail {

struct MethodInfo;

class RequestReader final: private boost::noncopyable {

public:
	RequestReader(boost::asio::ip::tcp::socket& socket, container::Buffer& buffer, http::Response& response, const MethodInfo& methodInfo);

	void Read(const coro::Context& context);
	bool NeedConnectionClose() const;

private:
	boost::asio::ip::tcp::socket& socket;
	container::Buffer& buffer;
	http::Response& response;
	const MethodInfo& methodInfo;
	bool needConnectionClose;
};

}
}
}
