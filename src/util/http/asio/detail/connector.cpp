#include "./connector.h"

#include "src/util/check/check.h"
#include "src/util/coro/future/executor.h"

http::asio::detail::Connector::Connector(boost::asio::ip::tcp::socket& socket, ResolveCache& resolveCache, coro::future::Executor& executor)
	: connect{std::bind(&Connector::Connect, this, std::placeholders::_1)}
	, socket{socket}
	, resolveCache{resolveCache}
	, executor{executor}
	, state{State::Disconnected}
	, future{executor.GetIoService()}
	, resolver{executor.GetIoService()}
	, commonCacheLine(nullptr)
{
}

void http::asio::detail::Connector::SetHost(std::string&& host) {
	if (this->host == host) {
		return;
	}
	const bool oldHostIsEmpty = this->host.empty();
	this->host = std::forward<std::string>(host);
	localCacheLine.generation = 0;
	commonCacheLine = this->host.empty() ? nullptr : &resolveCache.GetLine(this->host);
	if (oldHostIsEmpty) {
		Check(state == State::Disconnected);
		StartConnecting();
	} else if (this->host == "") {
		switch (state) {
			case State::Disconnected:
				CheckFalse();
				break;
			case State::Connecting:
				RestartConnecting();
				break;
			case State::Connected:
				Disconnect();
				break;
			case State::Error:
				state = State::Disconnected;
				break;
			default:
				CheckFalse();
		}
	} else {
		switch (state) {
			case State::Disconnected:
				CheckFalse();
				break;
			case State::Connecting:
				RestartConnecting();
				break;
			case State::Connected:
				Disconnect();
				StartConnecting();
				break;
			case State::Error:
				StartConnecting();
				break;
			default:
				CheckFalse();
		}
	}
}

http::asio::detail::Connector::State http::asio::detail::Connector::GetState() const {
	return state;
}

const std::string& http::asio::detail::Connector::GetHost() const
{
	Check(state == State::Connected);
	return host;
}

void http::asio::detail::Connector::Wait(const coro::Context& context) {
	Check(state == State::Connecting);
	future.Get(context);
}

void http::asio::detail::Connector::AttemptConnect(const coro::Context& context) {
	Check(state == State::Error);
	state = State::Connecting;
	Connect(context);
}

void http::asio::detail::Connector::Reconnect() {
	Disconnect();
	StartConnecting();
}

void http::asio::detail::Connector::Reconnect(const coro::Context& context) {
	Disconnect();
	state = State::Connecting;
	Connect(context);
}

void http::asio::detail::Connector::StartConnecting() {
	switch (state) {
		case State::Disconnected:
		case State::Error:
			break;
		case State::Connecting:
		case State::Connected:
		default:
			CheckFalse();
			break;
	}
	state = State::Connecting;
	executor.Execute(std::bind(std::cref(connect), std::placeholders::_1), future);
}

void http::asio::detail::Connector::Disconnect() {
	Check(state == State::Connected);
	ResetSocket();
	state = State::Disconnected;
}

void http::asio::detail::Connector::RestartConnecting() {
	Check(state == State::Connecting);
}

void http::asio::detail::Connector::Connect(const coro::Context& context) {
	bool resolveDone = false;
	for (;;) {
		if (host.empty()) {
			return;
		}
		bool needResolve = false;
		Check(commonCacheLine);
		if (commonCacheLine->generation == 0 || needResolve) {
			Resolve(context);
			resolveDone = true;
			if (commonCacheLine->generation == 0 || localCacheLine.endpoints != commonCacheLine->endpoints) {
				commonCacheLine->endpoints = localCacheLine.endpoints;
				localCacheLine.generation = ++commonCacheLine->generation;
			} 
		} else if (localCacheLine.generation < commonCacheLine->generation) {
			localCacheLine = *commonCacheLine;
		}
		boost::system::error_code ec;
		auto connectCondition = std::bind(&Connector::ConnectCondition, this, std::placeholders::_1, std::placeholders::_2);
		boost::asio::async_connect(socket, localCacheLine.endpoints.begin(), localCacheLine.endpoints.end(), std::move(connectCondition), context[ec]);
		if (localCacheLine.generation == 0) {
			// host changed
			if (!ec) {
				ResetSocket();
			}
			resolveDone = false;
			continue;
		}
		if (ec) {
			if (resolveDone) {
				state = State::Error;
				throw boost::system::system_error(ec);
			} else {
				needResolve = true;
				continue;
			}
		}
		break;
	}
	state = State::Connected;
}

http::asio::detail::Connector::Iterator http::asio::detail::Connector::ConnectCondition(const boost::system::error_code& /*ec*/, Iterator next) {
	return localCacheLine.generation == 0 ? localCacheLine.endpoints.end() : next;
}

void http::asio::detail::Connector::Resolve(const coro::Context& context) {
	static const std::string service{"http"};
	const boost::asio::ip::tcp::resolver::query query(host, service);
	boost::asio::ip::tcp::resolver::iterator begin{resolver.async_resolve(query, context)};
	boost::asio::ip::tcp::resolver::iterator end;
	std::copy(begin, end, std::back_inserter(localCacheLine.endpoints));
}

void http::asio::detail::Connector::ResetSocket() {
	socket.shutdown(socket.shutdown_both);
	socket.close();
}
