#pragma once

#include <boost/asio.hpp>
#include <boost/container/vector.hpp>

namespace http {
namespace asio {
namespace detail {

struct ResolveCacheLine final {
	using Endpoint = boost::asio::ip::tcp::resolver::iterator::value_type;
	using Endpoints = boost::container::vector<Endpoint>;
	using Iterator = Endpoints::iterator;

	size_t generation;
	Endpoints endpoints;

	ResolveCacheLine();
};

inline ResolveCacheLine::ResolveCacheLine()
	: generation(0)
{
}

}
}
}