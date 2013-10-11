#include "./resolve_cache.h"

http::asio::detail::ResolveCacheLine& http::asio::ResolveCache::GetLine(const std::string& host) {
	return cache[host];
}