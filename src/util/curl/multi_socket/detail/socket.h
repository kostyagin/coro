#pragma once

#include <curl/curl.h>

#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>

namespace curl {
namespace multi_socket {
namespace detail {

class SocketListener;

class Socket final: private boost::noncopyable {

public:
	Socket(boost::asio::io_service& ioService, curl::multi_socket::detail::SocketListener& listener);
	~Socket();

	curl_socket_t GetNativeHandle();
	bool Open(); // TCP IP4
	void SetPoll(bool pollIn, bool pollOut);

private:
	SocketListener& listener;
	boost::asio::ip::tcp::socket socket;
	bool pollIn;
	bool pollOut;
	int generation;
	int pollInGeneration;
	int pollOutGeneration;

	void PollIn();
	void PollOut();
	void Cancel();
	void PollInCallback(const boost::system::error_code& ec);
	void PollOutCallback(const boost::system::error_code& ec);
};

}
}
}
