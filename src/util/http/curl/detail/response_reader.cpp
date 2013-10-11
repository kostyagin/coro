#include "./response_reader.h"

#include "src/util/container/buffer.h"
#include "src/util/curl/easy.h"
#include "src/util/curl/check.h"
#include "src/util/http/response.h"

http::curl::detail::ResponseReader::ResponseReader(::curl::Easy& easy, container::Buffer& buffer, http::Response& response)
	: buffer(buffer)
	, response(response)
	, statusLineProcessed(false)
	, bodyStarted(false)
{
	buffer.Clear();
	response.headers.clear();
	response.body = container::Slice{};
	::curl::CheckCode(curl_easy_setopt(easy, CURLOPT_HEADERFUNCTION, &ResponseReader::CurlWriteHeaderCallback));
	::curl::CheckCode(curl_easy_setopt(easy, CURLOPT_WRITEHEADER, this));
	::curl::CheckCode(curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, &ResponseReader::CurlWriteBodyCallback));
	::curl::CheckCode(curl_easy_setopt(easy, CURLOPT_WRITEDATA, this));
}

size_t http::curl::detail::ResponseReader::ProcessCurlWriteHeaderCallback(void* ptr, size_t size, size_t nmemb) {
	const size_t length = size * nmemb;
	if (statusLineProcessed) {
		ProcessHeader(ptr, length);
	} else {
		ProcessStatusLine(ptr, length);
		statusLineProcessed = true;
	}
	return length;
}

void http::curl::detail::ResponseReader::ProcessStatusLine(void* /*ptr*/, size_t /*length*/) {
	// do nothing - result code will be obtained throught curl_easy_getinfo
}

void http::curl::detail::ResponseReader::ProcessHeader(void* ptr, size_t length) {
	// !!!
}


size_t http::curl::detail::ResponseReader::ProcessCurlWriteBodyCallback(char* /*ptr*/, size_t /*size*/, size_t /*nmemb*/) {
	// !!!
	return 0;
}

size_t http::curl::detail::ResponseReader::CurlWriteHeaderCallback(void* ptr, size_t size, size_t nmemb, ResponseReader* userdata) {
	return userdata->ProcessCurlWriteHeaderCallback(ptr, size, nmemb);
}

size_t http::curl::detail::ResponseReader::CurlWriteBodyCallback(char* ptr, size_t size, size_t nmemb, ResponseReader* userdata) {
	return userdata->ProcessCurlWriteBodyCallback(ptr, size, nmemb);
}