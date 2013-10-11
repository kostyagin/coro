#pragma once

#include <curl/curl.h>

namespace curl {

class Easy final {

public:
	Easy();
	~Easy();

	operator CURL*();

private:
	CURL* curl;
};

}
