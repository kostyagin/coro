#include "./check.h"

#include "src/util/check/check.h"

void curl::CheckCode(CURLcode code) {
	Check(code == CURLE_OK);
}

void curl::CheckCode(CURLMcode code) {
	Check(code == CURLM_OK);
}
