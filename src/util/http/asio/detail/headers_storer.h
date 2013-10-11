 #pragma once

#include "src/util/http/headers.h"

#include <boost/noncopyable.hpp>

namespace container {
class Buffer;
struct Slice;
};

namespace http {
namespace asio {
namespace detail {

// Сохраняет слайсы заголовков в нормализованном виде (не зависящем от памяти в Buffer)
// Денормализует в деструкторе
class HeadersStorer final: public boost::noncopyable {

public:
	HeadersStorer(container::Buffer& buffer, http::Headers<container::Slice>& headers);
	~HeadersStorer();

	void Store(container::Slice& name, container::Slice& value);

private:
	container::Buffer& buffer;
	http::Headers<container::Slice>& headers;

	void Normalize(container::Slice& slice);
	void Normalize(char*& value);
	void Denormalize();
	void Denormalize(http::Header<container::Slice>& header);
	void Denormalize(container::Slice& slice);
	void Denormalize(char*& value);

};

}
}
}
