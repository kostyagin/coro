#include "./socket.h"

#include "./socket_listener.h"

#include "src/util/check/check.h"

curl::multi_socket::detail::Socket::Socket(boost::asio::io_service& ioService, curl::multi_socket::detail::SocketListener& listener)
	: listener{listener}
	, socket{ioService}
	, pollIn{false}
	, pollOut{false}
	, generation{0}
{
}

curl::multi_socket::detail::Socket::~Socket() {
	boost::system::error_code ec;
	socket.shutdown(socket.shutdown_both, ec);
	socket.close(ec);
}

curl_socket_t curl::multi_socket::detail::Socket::GetNativeHandle() {
	return socket.native_handle();
}

bool curl::multi_socket::detail::Socket::Open() {
	boost::system::error_code ec;
	socket.open(boost::asio::ip::tcp::v4(), ec);
	return !ec;
}

void curl::multi_socket::detail::Socket::SetPoll(bool pollIn, bool pollOut) {
	if (this->pollIn && !pollIn || this->pollOut && !pollOut) {
		Cancel();
	}
	if (!this->pollIn && pollIn) {
		PollIn();
	}
	if (!this->pollOut && pollOut) {
		PollOut();
	}
	Check(this->pollIn == pollIn);
	Check(this->pollOut == pollOut);
}

void curl::multi_socket::detail::Socket::PollIn() {
	pollInGeneration = generation;
	pollIn = true;
	socket.async_read_some(boost::asio::null_buffers{}, std::bind(&Socket::PollInCallback, this, std::placeholders::_1));
}

void curl::multi_socket::detail::Socket::PollOut() {
	pollOutGeneration = generation;
	pollOut = true;
	socket.async_write_some(boost::asio::null_buffers{}, std::bind(&Socket::PollOutCallback, this, std::placeholders::_1));
}

void curl::multi_socket::detail::Socket::Cancel() {
	boost::system::error_code ec;
	++generation;
	pollIn = false;
	pollOut = false;
	socket.cancel(ec);
	Check(!ec);
}

void curl::multi_socket::detail::Socket::PollInCallback(const boost::system::error_code& ec) {
	pollIn = false;
	if (ec) {
		if (ec != boost::asio::error::operation_aborted) {
			listener.OnError(GetNativeHandle());
		}
	} else if (pollInGeneration == generation) {
		listener.OnReadAvailable(GetNativeHandle());
	}
}

void curl::multi_socket::detail::Socket::PollOutCallback(const boost::system::error_code& ec) {
	pollOut = false;
	if (ec) { 
		if (ec != boost::asio::error::operation_aborted) {
			listener.OnError(GetNativeHandle());
		}
	} else if (pollOutGeneration == generation) {
		listener.OnWriteAvailable(GetNativeHandle());
	}
}
