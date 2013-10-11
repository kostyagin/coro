#include "./request_body_writer.h"

#include "src/util/curl/check.h"
#include "src/util/curl/easy.h"

#include <curl/curl.h>

#include <algorithm>

http::curl::detail::RequestBodyWriter::RequestBodyWriter(::curl::Easy& easy, const boost::container::string& body)
	: body(body)
	, written(0)
{
	if (!body.empty()) {
		::curl::CheckCode(curl_easy_setopt(easy, CURLOPT_READFUNCTION, &RequestBodyWriter::CurlReadCallback));
		::curl::CheckCode(curl_easy_setopt(easy, CURLOPT_READDATA, this));
	}
}

size_t http::curl::detail::RequestBodyWriter::CurlReadCallbackProcessor(void* ptr, size_t size, size_t nmemb) {
	const size_t toCopy = (std::min)(size * nmemb, body.size() - written);
	std::memcpy(ptr, body.c_str() + written, toCopy);
	written += toCopy;
	return toCopy;
}

size_t http::curl::detail::RequestBodyWriter::CurlReadCallback(void* ptr, size_t size, size_t nmemb, RequestBodyWriter* userdata) {
	return userdata->CurlReadCallbackProcessor(ptr, size, nmemb);
}