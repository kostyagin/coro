#include "src/util/container/buffer.h"
#include "src/util/coro/execute/pooled_executor.h"
#include "src/util/coro/execute/spawn_executor.h"
#include "src/util/coro/future/executor.h"
#include "src/util/curl/init.h"
#include "src/util/debug_new/debug_new.h"
#include "src/util/http/asio/client.h"
#include "src/util/http/asio/resolve_cache/resolve_cache.h"
#include "src/util/http/request.h"
#include "src/util/http/response.h"

#include <boost/asio/steady_timer.hpp>

#include <iostream>

#include <cinttypes>

boost::asio::io_service ioService;
coro::function::PooledExecutor pooledExecutor{ioService};
coro::function::Executor& functionExecutor{pooledExecutor};
coro::future::Executor futureExecutor{functionExecutor};
http::asio::ResolveCache resolveCache;

struct Url {
	const char* host;
	const char* filename;
} urls[] = {
	{ "lenta.ru", "/" },
/*
	{ "mail.ru", "/" },
	{ "ya.ru", "/" },
	{ "www.boost.org", "/" },
	{ "en.wikipedia.org", "/wiki/Main_Page" },
	{ "en.cppreference.com", "/w/cpp/algorithm/move" },
	{ "www.football-russia.tv", "/2013/09/blog-post_1173.html" },
	{ "www.cplusplus.com", "/reference/algorithm/rotate/" },
	{ "www.youtube.com", "/watch?v=YQWIuvjyXqA" },
	{ "ibigdan.livejournal.com", "/" },
	{ "digg.com", "/" },
	{ "www.rbc.ru", "/" },
	{ "www.kasparov.ru", "/" },
	{ "hh.ru", "/" },
	{ "spacerangershd.com" , "/" },
	{ "www.google.com", "/doodles/finder/2013/Global" },
	{ "www.onlygirlvideos.com", "/" },
	{ "www.betfair.com", "/exchange/football" },
	{ "www.betexplorer.com", "/" },
	{ "www.moneypunter.com", "/" },
	{ "www.sport-express.ru", "/" },
	{ "www.football-russia.tv", "/" },
	{ "news.sportbox.ru", "/" }
*/
};


void TestDownload(const coro::Context& context, const Url& url) {
	http::asio::Client asioClient{resolveCache, futureExecutor};
	http::Client& client{asioClient};
	client.SetHost(url.host);
	http::Request request;
	request.filename = url.filename;
	container::Buffer buffer;
	http::Response response;
	boost::asio::steady_timer timer(ioService);
	for (;;) {
		client.PerformGet(context, request, buffer, response);
		const uint64_t size = response.body.end - response.body.begin;
		printf("%i %s%s page size: %" PRIu64 ", allocations count: %" PRIu64 "\n", response.resultCode, url.host, url.filename, size, debug_new::GetAllocationsCount());
		timer.expires_from_now(boost::chrono::seconds(2));
		timer.async_wait(context);
	}
}

void main() {
	curl::Init curlInit;
	for (size_t i = 0; i < sizeof(urls) / sizeof(urls[0]); ++i) {
		functionExecutor.Execute(std::bind(TestDownload, std::placeholders::_1, std::ref(urls[i])));	
	}
	ioService.run();
}