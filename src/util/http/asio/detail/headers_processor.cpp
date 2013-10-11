#include "./headers_processor.h"

#include "./headers_storer.h"

#include "src/util/check/check.h"
#include "src/util/container/slice/range.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/predicate.hpp>

http::asio::detail::HeadersProcessor::HeadersProcessor(HeadersStorer& headersStorer)
	: headersStorer{headersStorer}
	, status{0}
	, isContentChunked{false}
	, contentLength{static_cast<size_t>(-1)}
	, needConnectionClose{false}
{
}

void http::asio::detail::HeadersProcessor::ProcessStatusLine(container::Slice& line) {
	while (line.begin < line.end && !isspace(*line.begin)) {
		++line.begin;
	}
	status = std::strtol(++line.begin, 0, 10);
}

namespace {
void Trim(container::Slice& s) {
	while (s.begin < s.end && isspace(*s.begin)) {
		++s.begin;
	}
	if (s.begin == s.end) {
		return;
	}
	while (isspace(*(s.end - 1))) {
		--s.end;
	}
}
}

void http::asio::detail::HeadersProcessor::ProcessHeaderLine(container::Slice& line) {
	CheckWithMessage(!isspace(*line.begin), "headers continuations not supported");
	char* const colon = std::find(line.begin, line.end, ':');
	Check(colon != line.end);
	container::Slice name{line.begin, colon};
	container::Slice value{colon + 1, line.end};
	Trim(name);
	Trim(value);
	ProcessHeader(name, value);
}

int http::asio::detail::HeadersProcessor::GetStatus() const {
	return status;
}

bool http::asio::detail::HeadersProcessor::IsContentChunked() const {
	return isContentChunked;
}

size_t http::asio::detail::HeadersProcessor::GetContentLength() const {
	return contentLength;
}

bool http::asio::detail::HeadersProcessor::NeedConnectionClose() const {
	return needConnectionClose;
}

void http::asio::detail::HeadersProcessor::ProcessHeader(container::Slice& name, container::Slice& value) {
	Check(name.begin < name.end);
	boost::algorithm::to_lower(name);
	if (boost::algorithm::equals(name, "connection")) {
		return ProcessConnectionValue(value);
	} else if (boost::algorithm::equals(name, "transfer-encoding")) {
		return ProcessTransferEncodingValue(value);
	} else if (boost::algorithm::equals(name, "content-encoding")) {
		CheckFalseWithMessage("content-encoding unsupported");
	} else if (boost::algorithm::equals(name, "content-length")) {
		return ProcessContentLengthValue(value);
	}
	headersStorer.Store(name, value);
}

void http::asio::detail::HeadersProcessor::ProcessConnectionValue(container::Slice& value) {
	if (boost::algorithm::equals(value, "close")) {
		needConnectionClose = true;
	} else if (boost::algorithm::equals(value, "keep-alive")) {
		needConnectionClose = false;
	} else {
		CheckFalseWithMessage("unknown connection header value");
	}
}

void http::asio::detail::HeadersProcessor::ProcessTransferEncodingValue(container::Slice& value) {
	if (boost::algorithm::equals(value, "chunked")) {
		isContentChunked = true;
	} else {
		CheckFalseWithMessage("unsupported transfer encoding");
	}
}

void http::asio::detail::HeadersProcessor::ProcessContentLengthValue(container::Slice& value) {
	contentLength = strtol(value.begin, 0, 10);
}
