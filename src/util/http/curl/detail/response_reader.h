#pragma once

#include <boost/noncopyable.hpp>

namespace container {
class Buffer;
}

namespace curl {
class Easy;
}

namespace http {
struct Response;
}

namespace http {
namespace curl {
namespace detail {

class ResponseReader: private boost::noncopyable {

public:
	ResponseReader(::curl::Easy& easy, container::Buffer& buffer, http::Response& response);

private:
	container::Buffer& buffer;
	http::Response& response;
	bool statusLineProcessed;
	bool bodyStarted;

	size_t ProcessCurlWriteHeaderCallback(void* ptr, size_t size, size_t nmemb);
	size_t ProcessCurlWriteBodyCallback(char* ptr, size_t size, size_t nmemb);

	void ProcessStatusLine(void* ptr, size_t length);
	void ProcessHeader(void* ptr, size_t length);

	static size_t CurlWriteHeaderCallback(void* ptr, size_t size, size_t nmemb, ResponseReader* userdata);
	static size_t CurlWriteBodyCallback(char* ptr, size_t size, size_t nmemb, ResponseReader* userdata);
};

}
}
}
