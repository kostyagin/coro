#pragma once

#include "src/util/coro/core/context.h"

#include <curl/curl.h>

#include <boost/noncopyable.hpp>

namespace curl {

class Easy;

class Executor: public boost::noncopyable {

public:
	virtual CURLcode Execute(const coro::Context& context, curl::Easy& easy) = 0;
};

} 