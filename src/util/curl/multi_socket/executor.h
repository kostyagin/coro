#pragma once

#include "./detail/socket_listener.h"
#include "./detail/timer.h"

#include "src/util/curl/executor.h"
#include "src/util/curl/multi.h"

#include <unordered_map>

namespace curl {
namespace multi_socket {

namespace detail {
class Socket;
}

class Executor final: public curl::Executor, private curl::multi_socket::detail::SocketListener {

public:
	explicit Executor(coro::function::Executor& executor);

	boost::asio::io_service& GetIoService();

private:
	std::function<void(curl::Easy&)> doExecute;
	std::unordered_map<curl_socket_t, std::unique_ptr<curl::multi_socket::detail::Socket>> sockets;
	curl::Multi multi;
	coro::function::Executor& executor;
	curl::multi_socket::detail::Timer timer;
	int stillRunningCount;

	// curl::Executor
	CURLcode Execute(const coro::Context& context, curl::Easy& easy) override final;

	void DoExecute(curl::Easy& easy);
	int ProcessCurlMultiTimerCallback(CURLM* multi, long timeout);
	int ProcessCurlSocketCallback(CURL* easy, curl_socket_t s, int action, curl::multi_socket::detail::Socket* socketp);
	curl_socket_t ProcessCurlOpenSocketCallback(curlsocktype purpose, struct curl_sockaddr* address);
	int ProcessCurlCloseSocketCallback(curl_socket_t item);

	void OnTimer();

	// curl::multi_socket::SocketListener
	virtual void OnReadAvailable(curl_socket_t s) override final;
	virtual void OnWriteAvailable(curl_socket_t s) override final;
	virtual void OnError(curl_socket_t s) override final;	

	void PerformMultiActionAndProcessFinished(curl_socket_t s, int action);
	void ProcessFinishedTransfers();

	static int CurlMultiTimerCallback(CURLM* multi, long timeout, curl::multi_socket::Executor* executor);
	static int CurlSocketCallback(CURL* easy, curl_socket_t s, int action, curl::multi_socket::Executor* userp, curl::multi_socket::detail::Socket* socketp);
	static curl_socket_t CurlOpenSocketCallback(curl::multi_socket::Executor* executor, curlsocktype purpose, struct curl_sockaddr* address);
	static int CurlCloseSocketCallback(curl::multi_socket::Executor* executor, curl_socket_t item);

};

}
}
