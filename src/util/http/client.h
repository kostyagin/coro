#pragma once

#include "src/util/coro/core/context.h"

#include <boost/noncopyable.hpp>

#include <functional>
#include <string>

namespace container {
class Buffer;
}

namespace http {

struct Request;
struct Response;

class Client: private boost::noncopyable {

public:
	virtual void SetHost(std::string&& host) = 0;
	virtual void PerformGet(const coro::Context& context, const http::Request& request, container::Buffer& buffer, http::Response& response) = 0;
};

}