#pragma once

#include "./headers.h"

#include "src/util/container/slice/slice.h"

namespace http {

struct Response final {
	int resultCode;
	http::Headers<container::Slice> headers;
	container::Slice body;
};

} 