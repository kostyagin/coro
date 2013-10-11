#include "./multi.h"

#include "src/util/check/check.h"

curl::Multi::Multi()
	: curlm(curl_multi_init())
{
	Check(curlm);
}

curl::Multi::operator CURLM*() {
	return curlm;
}

curl::Multi::~Multi() {
	curl_multi_cleanup(curlm);
}
