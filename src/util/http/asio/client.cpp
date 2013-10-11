#include "./client.h"

#include "./detail/method_info.h"
#include "./detail/request_writer.h"
#include "./detail/response_reader.h"

#include "src/util/check/check.h"
#include "src/util/coro/future/executor.h"
#include "src/util/http/request.h"
#include "src/util/http/response.h"

#include <algorithm>
#include <functional>

http::asio::Client::Client(ResolveCache& resolveCache, coro::future::Executor& executor)
	: socket{executor.GetIoService()}
	, connector{socket, resolveCache, executor}
{
}

void http::asio::Client::SetHost(std::string&& host) {
	connector.SetHost(std::forward<std::string>(host));	
}

void http::asio::Client::PerformGet(const coro::Context& context, const http::Request& request, container::Buffer& buffer, http::Response& response) {
	static const detail::MethodInfo methodInfo{"GET", false, true};
	Perform(context, request, buffer, response, methodInfo);
}

void http::asio::Client::Perform(const coro::Context& context, const http::Request& request, container::Buffer& buffer, http::Response& response, const detail::MethodInfo& methodInfo) {
	EnsureConnection(context);
	bool needConnectionClose = false;
	try {
		detail::RequestWriter writer{socket, connector.GetHost(), buffer, request, methodInfo};
		writer.Write(context);
		if (CheckEof(context)) {
			connector.Reconnect(context);
			writer.Write(context);
		}
		detail::RequestReader reader{socket, buffer, response, methodInfo};
		reader.Read(context);
		needConnectionClose = reader.NeedConnectionClose();
	} catch (...) {
		connector.Reconnect();
		throw;
	}
	if (needConnectionClose) {
		connector.Reconnect();
	}
}

void http::asio::Client::EnsureConnection(const coro::Context& context) {
	switch (connector.GetState()) {
		case detail::Connector::State::Connecting:
			connector.Wait(context);
			break;
		case detail::Connector::State::Connected:
			break;
		case detail::Connector::State::Error:
			connector.AttemptConnect(context);
			break;
		case detail::Connector::State::Disconnected:
		default:
			CheckFalse();
			break;		
	}
}

bool http::asio::Client::CheckEof(const coro::Context& context) {
	boost::system::error_code ec;
	char a;
	socket.async_receive(boost::asio::buffer(&a, 1), boost::asio::ip::tcp::socket::message_peek, context[ec]);
	return !!ec;
}
