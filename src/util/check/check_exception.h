#pragma once

#include <boost/container/string.hpp>
#include <boost/noncopyable.hpp>

#include <stdexcept>

class CheckException final: public std::logic_error {

public:
	CheckException(const char* condition, const boost::container::string& message, const char* filename, int line);
};