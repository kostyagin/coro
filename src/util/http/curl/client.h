#pragma once

#include "src/util/curl/easy.h"
#include "src/util/http/client.h"

namespace curl {
class Executor;
}

namespace http {
namespace curl {

class Client final: public http::Client {

public:
	explicit Client(::curl::Executor& executor);

private:
	::curl::Executor& executor;
	::curl::Easy easy;
	std::string host;

	// http::Client
	virtual void SetHost(std::string&& host) override final;
	virtual void PerformGet(const coro::Context& context, const http::Request& request, container::Buffer& buffer, http::Response& response) override final;
};

}
}
