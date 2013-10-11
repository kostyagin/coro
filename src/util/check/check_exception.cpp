#include "./check_exception.h"

#include <boost/format.hpp>

namespace
{

std::string ConstructErrorString(const char* condition, const boost::container::string& message, const char* filename, int line) {
	return str(boost::format("check failed in %1%, %2%: %3%%4%%5%") % filename % line % condition
		% (message.empty() ? "" : " - ") % (message.empty() ? "" : message));
}

}

CheckException::CheckException(const char* condition, const boost::container::string& message, const char* filename, int line)
	: std::logic_error(ConstructErrorString(condition, message, filename, line))
{
}
