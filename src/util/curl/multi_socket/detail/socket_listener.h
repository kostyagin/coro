#pragma once

#include <curl/curl.h>

namespace curl {
namespace multi_socket {
namespace detail {

class SocketListener {

public:
	virtual ~SocketListener() {}

	virtual void OnReadAvailable(curl_socket_t s) = 0;
	virtual void OnWriteAvailable(curl_socket_t s) = 0;
	virtual void OnError(curl_socket_t s) = 0;
};

}
}
}
