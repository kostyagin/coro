#pragma once

#include "src/util/http/asio/resolve_cache/resolve_cache.h"

#include "src/util/coro/core/function.h"
#include "src/util/coro/future/future.h"

#include <boost/asio.hpp>
#include <boost/container/vector.hpp>
#include <boost/noncopyable.hpp>

#include <string>

namespace coro {
namespace future {
class Executor;
}
}

namespace http {
namespace asio {
namespace detail {

class Connector final: private boost::noncopyable {
public:
	enum class State {
		Disconnected,
		Connecting,
		Connected,
		Error
	};

	Connector(boost::asio::ip::tcp::socket& socket, ResolveCache& resolveCache, coro::future::Executor& executor);

	// Изменить хост, к которому нужно коннектиться. Если пустая строка - разорвать все соединения.
	void SetHost(std::string&& host);
	// Вернуть текущее состояние
	State GetState() const;
	// Может быть вызвано только в состоянии Connected, чтобы получить имя хоста, к которому установлено соединение
	const std::string& GetHost() const;
	// Вызывается в состоянии Connecting, для того чтобы дождаться завершения.
	// Если переходит в состояние Error, то выкидывает исключение
	void Wait(const coro::Context& context);
	// Вызывается в состоянии Error для новой попытки соединения
	// Кидает ислючение, если неуспешно
	void AttemptConnect(const coro::Context& context);
	// Вызывается в состоянии Connected для разрыва соединения и установки нового
	void Reconnect();
	void Reconnect(const coro::Context& context);

private:
	coro::Function connect;
	boost::asio::ip::tcp::socket& socket;
	ResolveCache& resolveCache;
	coro::future::Executor& executor;
	std::string host;
	State state;
	coro::Future<void> future;
	boost::asio::ip::tcp::resolver resolver;
	ResolveCacheLine localCacheLine;
	ResolveCacheLine* commonCacheLine;
	using Iterator = ResolveCacheLine::Iterator;
	
	void StartConnecting();
	void Disconnect();
	void RestartConnecting();
	void Connect(const coro::Context& context);
	void Resolve(const coro::Context& context);
	Iterator ConnectCondition(const boost::system::error_code& ec, Iterator next);
	void ResetSocket();
};

}
}
}
