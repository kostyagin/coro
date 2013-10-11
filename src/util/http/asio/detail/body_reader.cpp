#include "./body_reader.h"

#include "./chunked_body_reader.h"

#include "src/util/check/check.h"
#include "src/util/container/buffer.h"

http::asio::detail::BodyReader::BodyReader(boost::asio::ip::tcp::socket& socket, container::Buffer& buffer, size_t headersLength)
	: socket{socket}
	, buffer{buffer}
	, headersLength{headersLength}
{
}

container::Slice http::asio::detail::BodyReader::Read(const coro::Context& context, bool isChunked, size_t contentLength, bool canHaveBody, bool& connectionClose) {
	if (isChunked) {
		ReadChunked(context, connectionClose);
	} else if (contentLength != static_cast<size_t>(-1)) {
		ReadSolid(context, contentLength, connectionClose);
	} else if (!canHaveBody) {
		ReadEmpty(context);
	} else if (connectionClose) {
		ReadUntilEof(context);
	} else {
		CheckFalseWithMessage("Unknown body length");
		ReadEmpty(context);
	}
	return container::Slice{buffer.GetPlace() + headersLength, buffer.GetPlace() + buffer.GetSize()};
}

void http::asio::detail::BodyReader::ReadChunked(const coro::Context& context, bool& connectionClose) {
	ChunkedBodyReader reader{socket, buffer, headersLength};
	reader.Read(context, connectionClose);
}

void http::asio::detail::BodyReader::ReadSolid(const coro::Context& context, size_t contentLength, bool& connectionClose) {
	const size_t restPartSize = contentLength - (buffer.GetSize() - headersLength);
	buffer.ReserveForAppend(restPartSize);
	boost::system::error_code ec;
	coro::Context ecContext{context[ec]};
	const size_t readed = boost::asio::async_read(socket, boost::asio::buffer(buffer.GetPlace() + buffer.GetSize(), restPartSize), ecContext);
	if (ec == boost::asio::error::eof) {
		connectionClose = true;
	} else if (ec) {
		throw boost::system::system_error(ec);
	}
	Check(readed == restPartSize);
	buffer.Resize(headersLength + contentLength);
}

void http::asio::detail::BodyReader::ReadEmpty(const coro::Context& /*context*/) {
	Check(headersLength == buffer.GetSize());
}

void http::asio::detail::BodyReader::ReadUntilEof(const coro::Context& context) {
	boost::system::error_code ec;
	coro::Context ecContext = context[ec];
	do {
		auto b = boost::asio::buffer(buffer.GetPlace() + buffer.GetSize(), buffer.GetCapacity() - buffer.GetSize());
		const size_t readed = boost::asio::async_read(socket, std::move(b), ecContext);
		buffer.Resize(buffer.GetSize() + readed);
	} while (!ec);
	if (ec == boost::asio::error::eof) {
		return;
	} else {
		throw boost::system::system_error(ec);
	}
}
