#pragma once

#include "./detail/connector.h"

#include "src/util/http/client.h"
#include "src/util/http/headers.h"

#include <boost/asio.hpp>

namespace http {
namespace asio {

class ResolveCache;

namespace detail {
struct MethodInfo;
}

class Client final: public http::Client {

public:
	Client(ResolveCache& resolveCache, coro::future::Executor& executor);

private:
	boost::asio::ip::tcp::socket socket;
	detail::Connector connector;

	// http::Client
	virtual void SetHost(std::string&& host) override final;
	virtual void PerformGet(const coro::Context& context, const http::Request& request, container::Buffer& buffer, http::Response& response) override final;

	void Perform(const coro::Context& context, const http::Request& request, container::Buffer& buffer, http::Response& response, const detail::MethodInfo& methodInfo);
	void EnsureConnection(const coro::Context& context);

	bool CheckEof(const coro::Context& context);
};

}
}
