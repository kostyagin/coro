#pragma once

#include "src/util/coro/core/context.h"

#include <boost/asio.hpp>
#include <boost/container/string.hpp>
#include <boost/noncopyable.hpp>

namespace container {
class Buffer;
}

namespace http {
struct Request;
}

namespace http {
namespace asio {
namespace detail {

struct MethodInfo;

class RequestWriter final: private boost::noncopyable {

public:
	RequestWriter(boost::asio::ip::tcp::socket& socket, const std::string& host, container::Buffer& buffer, const http::Request& request, const MethodInfo& methodInfo);

	void Write(const coro::Context& context);

private:
	boost::asio::ip::tcp::socket& socket;
	const std::string& host;
	container::Buffer& buffer;
	const http::Request& request;
	const MethodInfo& methodInfo;
};

}
}
}