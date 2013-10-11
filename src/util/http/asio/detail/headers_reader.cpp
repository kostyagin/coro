#include "./headers_reader.h"

#include "./headers_processor.h"

#include "src/util/check/check.h"
#include "src/util/container/buffer.h"

http::asio::detail::HeadersReader::HeadersReader(boost::asio::ip::tcp::socket& socket, container::Buffer& buffer, HeadersProcessor& processor)
	: socket{socket}
	, buffer{buffer}
	, processor{processor}
	, readed{0}
	, headersReaded{false}
{
}

size_t http::asio::detail::HeadersReader::Read(const coro::Context& context) {
	buffer.Clear();
	nextLineBegin = buffer.GetPlace();
	do {
		lastBytesTransferred = 0;
		char* const readTo = buffer.GetPlace() + buffer.GetSize();
		const size_t bytesAvailable = buffer.GetCapacity() - buffer.GetSize();
		auto completion = std::bind(&HeadersReader::CompletionCondition, this, std::placeholders::_1, std::placeholders::_2);
		readed += boost::asio::async_read(socket, boost::asio::buffer(readTo, bytesAvailable), std::move(completion), context);
		const size_t offset = nextLineBegin - buffer.GetPlace();
		buffer.Resize(readed);
		nextLineBegin = buffer.GetPlace() + offset;
	} while (!headersReaded);
	return nextLineBegin - buffer.GetPlace();
}

size_t http::asio::detail::HeadersReader::CompletionCondition(const boost::system::error_code& error, size_t bytesTransferred) {
	if (error) {
		return 0;
	}
	static const char toFind[] = "\r\n";
	static const size_t toFindLength = sizeof(toFind) / sizeof(toFind[0]) - 1;
	char* const findTo = buffer.GetPlace() + readed + bytesTransferred;
	char* findFrom = std::max(nextLineBegin, buffer.GetPlace() + readed + lastBytesTransferred - (toFindLength - 1));
	lastBytesTransferred = bytesTransferred;
	for (;;) {
		Check(findFrom <= findTo);
		char* const found = std::search(findFrom, findTo, toFind, toFind + toFindLength);
		if (found == findTo) {
			// на середине очередной строки, читаем дальше
			return buffer.GetCapacity() - buffer.GetSize() - bytesTransferred;
		} else if (found == findFrom) {
			// пустая строка - конец заголовков
			nextLineBegin = found + toFindLength;
			headersReaded = true;
			return 0;
		} else {
			// нашли очередную строку
			container::Slice line{nextLineBegin, found};
			if (nextLineBegin == buffer.GetPlace()) {
				processor.ProcessStatusLine(line);
			} else {
				processor.ProcessHeaderLine(line);
			}
			findFrom = found + toFindLength;
			nextLineBegin = findFrom;
		}
	}	
}