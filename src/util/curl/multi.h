#pragma once

#include <curl/curl.h>

#include <boost/noncopyable.hpp>

namespace curl {

class Multi final: public boost::noncopyable {

public:
	Multi();
	~Multi();

	operator CURLM*();

private:
	CURLM* curlm;
};

}
