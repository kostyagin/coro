#include "./chunked_body_reader.h"

#include "src/util/container/buffer.h"

http::asio::detail::ChunkedBodyReader::ChunkedBodyReader(boost::asio::ip::tcp::socket& socket, container::Buffer& buffer, size_t headersLength)
	: socket{socket}
	, buffer{buffer}
	, compactor{buffer, headersLength}
{
}

void http::asio::detail::ChunkedBodyReader::Read(const coro::Context& context, bool& connectionClose) {
	boost::system::error_code ec;
	coro::Context ecContext{context[ec]};	
	while (!ec && !compactor.IsFinished()) {
		readingFrom = compactor.GetUsedSpace();
		auto b = boost::asio::buffer(buffer.GetPlace() + readingFrom, buffer.GetCapacity() - readingFrom);
		auto c = std::bind(&ChunkedBodyReader::CompletionCondition, this, std::placeholders::_1, std::placeholders::_2);
		lastBytesTransferred = 0;
		boost::asio::async_read(socket, std::move(b), std::move(c), ecContext);
	}
	buffer.Resize(compactor.GetUsedSpace());
	if (!ec) {
		return;
	}
	connectionClose = true;
	if (ec != boost::asio::error::eof || !compactor.IsFinished()) {
		throw boost::system::system_error(ec);
	}
}

size_t http::asio::detail::ChunkedBodyReader::CompletionCondition(const boost::system::error_code& error, size_t bytesTransferred) {
	if (error) {
		return 0;
	}
	buffer.Resize(readingFrom + bytesTransferred);
	compactor.AddChunk(readingFrom + lastBytesTransferred);
	lastBytesTransferred = bytesTransferred;
	return compactor.IsFinished() ? 0 : buffer.GetCapacity() - readingFrom - bytesTransferred;
}