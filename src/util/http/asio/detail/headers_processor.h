#pragma once

#include "src/util/container/slice/slice.h"

#include <boost/noncopyable.hpp>

namespace http {
namespace asio {
namespace detail {

class HeadersStorer;

class HeadersProcessor final: private boost::noncopyable {

public:
	explicit HeadersProcessor(HeadersStorer& headersStorer);

	void ProcessStatusLine(container::Slice& line);
	void ProcessHeaderLine(container::Slice& line);

	int GetStatus() const;
	bool IsContentChunked() const;
	size_t GetContentLength() const;
	bool NeedConnectionClose() const;

private:
	HeadersStorer& headersStorer;
	int status;
	bool isContentChunked;
	size_t contentLength;
	bool needConnectionClose;

	void ProcessHeader(container::Slice& name, container::Slice& value);
	void ProcessConnectionValue(container::Slice& value);
	void ProcessTransferEncodingValue(container::Slice& value);
	void ProcessContentLengthValue(container::Slice& value);
};

}
}
}