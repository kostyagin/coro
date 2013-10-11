#include "./easy.h"

#include "src/util/check/check.h"

curl::Easy::Easy()
	: curl(curl_easy_init())
{
	Check(curl != nullptr);
}

curl::Easy::operator CURL*() {
	return curl;
}

curl::Easy::~Easy() {
	curl_easy_cleanup(curl);
}
