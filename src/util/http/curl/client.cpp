#include "./client.h"

#include "./detail/request_body_writer.h"
#include "./detail/response_reader.h"

#include "src/util/container/buffer.h"
#include "src/util/curl/check.h"
#include "src/util/curl/executor.h"
#include "src/util/curl/slist.h"
#include "src/util/http/request.h"

http::curl::Client::Client(::curl::Executor& executor)
	: executor{executor}
{
}

void http::curl::Client::SetHost(std::string&& host) {
	this->host = std::forward<std::string>(host);
}

void http::curl::Client::PerformGet(const coro::Context& context, const http::Request& request, container::Buffer& buffer, http::Response& response) {
	curl_easy_reset(easy);
	buffer.Clear();
	buffer.Append("http://").Append(host).Append(request.filename).Append('\0');
	::curl::CheckCode(curl_easy_setopt(easy, CURLOPT_URL, buffer.GetPlace()));
	::curl::SList slist;
	for (const auto& header: request.headers) {
		buffer.Clear();
		buffer.Append(header.name).Append(':').Append(header.value).Append('\0');
		slist.Append(buffer.GetPlace());
	}
	if (!slist.Empty()) {
		::curl::CheckCode(curl_easy_setopt(easy, CURLOPT_HTTPHEADER, static_cast<struct curl_slist*>(slist)));
	}
	const detail::RequestBodyWriter requestBodyWriter(easy, request.body);
	const detail::ResponseReader responseReader(easy, buffer, response);
	const CURLcode result = executor.Execute(context, easy);
	// !!! convert result to exception if error occured
	// !!! extract http result code to response.resultCode
}
