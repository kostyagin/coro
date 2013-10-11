#include "./executor.h"

#include <boost/noncopyable.hpp>

#include "./detail/socket.h"

#include "src/util/coro/condition/condition.h"
#include "src/util/coro/execute/executor.h"
#include "src/util/curl/check.h"
#include "src/util/curl/easy.h"

namespace {

struct EasyData: private boost::noncopyable {
	CURLcode result;
	coro::Condition condition;

	EasyData(boost::asio::io_service& ioService) 
		: result(CURL_LAST)
		, condition(ioService)
	{
	}
};

void StoreData(CURL* easy, EasyData& data) {
	curl::CheckCode(curl_easy_setopt(easy, CURLOPT_PRIVATE, &data));
}

EasyData& LoadData(CURL* easy) {
	EasyData* result = 0;
	curl::CheckCode(curl_easy_getinfo(easy, CURLINFO_PRIVATE, &result));
	return *result;
}

}

curl::multi_socket::Executor::Executor(coro::function::Executor& executor)
	: doExecute{std::bind(&Executor::DoExecute, this, std::placeholders::_1)}
	, executor{executor}
	, timer{executor, std::bind(&Executor::OnTimer, this)}
	, stillRunningCount{0}
{
	CheckCode(curl_multi_setopt(multi, CURLMOPT_SOCKETFUNCTION, &Executor::CurlSocketCallback));
	CheckCode(curl_multi_setopt(multi, CURLMOPT_SOCKETDATA, this));
	CheckCode(curl_multi_setopt(multi, CURLMOPT_TIMERFUNCTION, &Executor::CurlMultiTimerCallback));
	CheckCode(curl_multi_setopt(multi, CURLMOPT_TIMERDATA, this));
}

boost::asio::io_service& curl::multi_socket::Executor::GetIoService() {
	return executor.GetIoService();
}

CURLcode curl::multi_socket::Executor::Execute(const coro::Context& context, curl::Easy& easy) {
	CheckCode(curl_easy_setopt(easy, CURLOPT_OPENSOCKETFUNCTION, CurlOpenSocketCallback));
	CheckCode(curl_easy_setopt(easy, CURLOPT_OPENSOCKETDATA, this));
	CheckCode(curl_easy_setopt(easy, CURLOPT_CLOSESOCKETFUNCTION, CurlCloseSocketCallback));
	CheckCode(curl_easy_setopt(easy, CURLOPT_CLOSESOCKETDATA, this));
	boost::asio::io_service& ioService{executor.GetIoService()};
	EasyData data{ioService};
	StoreData(easy, data);
	ioService.post(std::bind(std::cref(doExecute), std::ref(easy)));
	data.condition.Wait(context);
	return data.result;
}

void curl::multi_socket::Executor::DoExecute(curl::Easy& easy) {
	curl_multi_add_handle(multi, easy);
}

int curl::multi_socket::Executor::ProcessCurlMultiTimerCallback(CURLM* multi, long timeout) {
	Check(multi == this->multi);
	timer.Cancel();
	switch (timeout) {
		case -1:
			// do nothing
			break;
		case 0:
			OnTimer();
			break;
		default:
			Check(timeout > 0);
			timer.SetTimeout(timeout);
			break;
	}
	return 0;
}

int curl::multi_socket::Executor::ProcessCurlSocketCallback(CURL* /*easy*/, curl_socket_t s, int action, curl::multi_socket::detail::Socket* socket) {
	Check(s == socket->GetNativeHandle());
	const bool pollIn = action == CURL_POLL_IN || action == CURL_POLL_INOUT;
	const bool pollOut = action == CURL_POLL_OUT || action == CURL_POLL_INOUT;
	socket->SetPoll(pollIn, pollOut);
	return 0;
}

curl_socket_t curl::multi_socket::Executor::ProcessCurlOpenSocketCallback(curlsocktype purpose, struct curl_sockaddr* address) {
	if (purpose != CURLSOCKTYPE_IPCXN || address->family != AF_INET) {
		return CURL_SOCKET_BAD; // Only TCP IPv4 supported
	}
	std::unique_ptr<curl::multi_socket::detail::Socket> socket{new curl::multi_socket::detail::Socket{executor.GetIoService(), *this}};
	if (!socket->Open()) {
		return CURL_SOCKET_BAD;
	}
	const curl_socket_t handle{socket->GetNativeHandle()};
	CheckCode(curl_multi_assign(multi, handle, socket.get()));
	sockets[socket->GetNativeHandle()] = std::move(socket);
	return handle;
}

int curl::multi_socket::Executor::ProcessCurlCloseSocketCallback(curl_socket_t item) {
	sockets.erase(item);
	return 0;
}

void curl::multi_socket::Executor::OnTimer() {
	PerformMultiActionAndProcessFinished(CURL_SOCKET_TIMEOUT, 0);
}

void curl::multi_socket::Executor::OnReadAvailable(curl_socket_t s) {
	PerformMultiActionAndProcessFinished(s, CURL_CSELECT_IN);
}

void curl::multi_socket::Executor::OnWriteAvailable(curl_socket_t s) {
	PerformMultiActionAndProcessFinished(s, CURL_CSELECT_OUT);
}

void curl::multi_socket::Executor::OnError(curl_socket_t s) {
	PerformMultiActionAndProcessFinished(s, CURL_CSELECT_ERR);
}

void curl::multi_socket::Executor::PerformMultiActionAndProcessFinished(curl_socket_t s, int action) {
	const int oldStillRuningCount = stillRunningCount;
	CheckCode(curl_multi_socket_action(multi, s, action, &stillRunningCount));
	ProcessFinishedTransfers();
	Check(stillRunningCount >= 0);
	if (oldStillRuningCount == 0) {
		timer.Cancel();
	}
}

void curl::multi_socket::Executor::ProcessFinishedTransfers() {
	int messagesInQueue;
	while (CURLMsg* msg = curl_multi_info_read(multi, &messagesInQueue)) {
		CURL* const easy = msg->easy_handle;
		const CURLcode result = msg->data.result;
		Check(msg->msg == CURLMSG_DONE);
		EasyData& data = LoadData(easy);
		data.result = result;
		data.condition.NotifyOne();
	}
}

int curl::multi_socket::Executor::CurlMultiTimerCallback(CURLM* multi, long timeout, curl::multi_socket::Executor* executor) {
	return executor->ProcessCurlMultiTimerCallback(multi, timeout);
}

int curl::multi_socket::Executor::CurlSocketCallback(CURL* easy, curl_socket_t s, int action, curl::multi_socket::Executor* executor, curl::multi_socket::detail::Socket* socket) {
	return executor->ProcessCurlSocketCallback(easy, s, action, socket);
}

curl_socket_t curl::multi_socket::Executor::CurlOpenSocketCallback(curl::multi_socket::Executor* executor, curlsocktype purpose, struct curl_sockaddr* address) {
	return executor->ProcessCurlOpenSocketCallback(purpose, address);	
}

int curl::multi_socket::Executor::CurlCloseSocketCallback(curl::multi_socket::Executor* executor, curl_socket_t item) {
	return executor->ProcessCurlCloseSocketCallback(item);
}
