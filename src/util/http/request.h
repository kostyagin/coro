#pragma once

#include "./headers.h"

#include <boost/container/string.hpp>

namespace http {

struct Request final {
	boost::container::string filename;
	http::Headers<boost::container::string> headers;
	boost::container::string body;
};

}