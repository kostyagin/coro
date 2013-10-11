#include "./headers_storer.h"

#include "src/util/container/buffer.h"
#include "src/util/container/slice/slice.h"

http::asio::detail::HeadersStorer::HeadersStorer(container::Buffer& buffer, http::Headers<container::Slice>& headers)
	: buffer{buffer}
	, headers{headers}
{
	headers.clear();
}

http::asio::detail::HeadersStorer::~HeadersStorer() {
	Denormalize();
}

void http::asio::detail::HeadersStorer::Store(container::Slice& name, container::Slice& value) {
	Normalize(name);
	Normalize(value);
	const Header<container::Slice> header{name, value};
	headers.push_back(header);
}

void http::asio::detail::HeadersStorer::Normalize(container::Slice& slice) {
	Normalize(slice.begin);
	Normalize(slice.end);
}

void http::asio::detail::HeadersStorer::Normalize(char*& value) {
	static char* const null = 0;
	value = null + (value - buffer.GetPlace());
}

void http::asio::detail::HeadersStorer::Denormalize() {
	for (auto& header: headers) {
		Denormalize(header);
	}
}

void http::asio::detail::HeadersStorer::Denormalize(http::Header<container::Slice>& header) {
	Denormalize(header.name);
	Denormalize(header.value);
}

void http::asio::detail::HeadersStorer::Denormalize(container::Slice& slice) {
	Denormalize(slice.begin);
	Denormalize(slice.end);
}

void http::asio::detail::HeadersStorer::Denormalize(char*& value) {
	static char* null = 0;
	value = buffer.GetPlace() + (value - null);
}
