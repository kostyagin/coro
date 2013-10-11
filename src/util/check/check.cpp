#include "./check.h"

#include "./check_exception.h"

namespace
{

void ThrowCheckException(const char* condition, const boost::container::string& message, const char* filename, int line) {
	throw CheckException(condition, message, filename, line);
}

}

void Fail(const char* condition, const boost::container::string& message, const char* filename, int line) {
	ThrowCheckException(condition, message, filename, line);
}