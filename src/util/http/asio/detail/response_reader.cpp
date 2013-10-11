#include "./response_reader.h"

#include "./body_reader.h"
#include "./headers_processor.h"
#include "./headers_reader.h"
#include "./headers_storer.h"
#include "./method_info.h"

#include "src/util/container/buffer.h"
#include "src/util/check/check.h"
#include "src/util/http/response.h"

#include <algorithm>

http::asio::detail::RequestReader::RequestReader(boost::asio::ip::tcp::socket& socket, container::Buffer& buffer, http::Response& response, const MethodInfo& methodInfo)
	: socket{socket}
	, buffer{buffer}
	, response{response}
	, methodInfo{methodInfo}
	, needConnectionClose{false}
{
}

void http::asio::detail::RequestReader::Read(const coro::Context& context) {
	HeadersStorer headersStorer{buffer, response.headers};
	HeadersProcessor headersProcessor{headersStorer};
	HeadersReader headersReader{socket, buffer, headersProcessor};
	const size_t headersLength = headersReader.Read(context);
	response.resultCode = headersProcessor.GetStatus();
	needConnectionClose = headersProcessor.NeedConnectionClose();
	BodyReader bodyReader(socket, buffer, headersLength);
	response.body = bodyReader.Read(context, headersProcessor.IsContentChunked(), headersProcessor.GetContentLength(), methodInfo.responseCanHaveBody, needConnectionClose);	
}

bool http::asio::detail::RequestReader::NeedConnectionClose() const {
	return needConnectionClose;
}