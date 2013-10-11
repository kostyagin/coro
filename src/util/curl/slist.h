#pragma once

#include <curl/curl.h>

#include <boost/noncopyable.hpp>

namespace curl {

class SList: public boost::noncopyable {

public:
	SList();
	~SList();

	operator struct curl_slist*();

	void Append(const char* string);
	bool Empty() const;

private:
	struct curl_slist* slist;
};

}
