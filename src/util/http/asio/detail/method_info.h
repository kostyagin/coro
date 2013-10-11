#pragma once

#include <boost/container/string.hpp>

namespace http {
namespace asio {
namespace detail {

struct MethodInfo final {
	boost::container::string name;
	bool requestCanHaveBody;
	bool responseCanHaveBody;
};
 
}
}
}
