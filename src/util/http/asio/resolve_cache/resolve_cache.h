#pragma once

#include "./detail/resolve_cache_line.h"

#include <string>
#include <unordered_map>

namespace http {
namespace asio {

class ResolveCache final {

public:
	detail::ResolveCacheLine& GetLine(const std::string& host);

private:
	std::unordered_map<std::string, detail::ResolveCacheLine> cache;
};

}
}