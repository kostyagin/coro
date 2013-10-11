#pragma once

#include <boost/container/string.hpp>
#include <boost/noncopyable.hpp>

namespace curl {
class Easy;
}

namespace http {
namespace curl {
namespace detail {

class RequestBodyWriter: private boost::noncopyable {

public:
	RequestBodyWriter(::curl::Easy& easy, const boost::container::string& body);

private:
	const boost::container::string& body;
	size_t written;

	size_t CurlReadCallbackProcessor(void* ptr, size_t size, size_t nmemb);

	static size_t CurlReadCallback(void* ptr, size_t size, size_t nmemb, RequestBodyWriter* userdata);
};

}
}
}
