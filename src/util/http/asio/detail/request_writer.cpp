#include "./request_writer.h"

#include "./method_info.h"

#include "src/util/check/check.h"
#include "src/util/container/buffer.h"
#include "src/util/http/request.h"

http::asio::detail::RequestWriter::RequestWriter(boost::asio::ip::tcp::socket& socket, const std::string& host, container::Buffer& buffer, const http::Request& request, const MethodInfo& methodInfo)
	: socket{socket}
	, host{host}
	, buffer{buffer}
	, request{request}
	, methodInfo{methodInfo}
{
}

void http::asio::detail::RequestWriter::Write(const coro::Context& context) {
	buffer.Clear();
	buffer.Append(methodInfo.name).Append(' ').Append(request.filename).Append(" HTTP/1.1\r\n");
	buffer.Append("Host: ").Append(host).Append("\r\n");
	for (const auto& header: request.headers) {
		buffer.Append(header.name).Append(":").Append(header.value).Append("\r\n");
	}
	CheckWithMessage(!methodInfo.requestCanHaveBody, "Http requests with body not supported yet");
	buffer.Append("\r\n");
	boost::asio::async_write(socket, boost::asio::buffer(buffer.GetPlace(), buffer.GetSize()), context);
}