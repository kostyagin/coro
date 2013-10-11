#pragma once

#include "./header.h"

#include <boost/container/vector.hpp>

namespace http {

template <typename T> using Headers = boost::container::vector<http::Header<T>>;

}