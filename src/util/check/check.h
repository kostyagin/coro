#pragma once

#include <boost/container/string.hpp>
#include <boost/preprocessor/stringize.hpp>

void Fail(const char* condition, const boost::container::string& message, const char* filename, int line);

#ifdef HIDE_CHECK_CONDITIONS
#define STRINGIZE_CONDITION(condition) "hidden condition"
#else
#define STRINGIZE_CONDITION(condition) BOOST_PP_STRINGIZE(condition)
#endif

#ifdef DONT_CHECK
#else
#define CheckFalse() Fail("false", "", __FILE__, __LINE__)
#define Check(condition) if (!(condition)) Fail(STRINGIZE_CONDITION(condition), "", __FILE__, __LINE__)
#define CheckFalseWithMessage(message) Fail("false", message, __FILE__, __LINE__)
#define CheckWithMessage(condition, message) if (!(condition)) Fail(STRINGIZE_CONDITION(condition), message, __FILE__, __LINE__)
#endif