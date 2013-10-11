#pragma once

namespace http {

template <typename T>
struct Header final {
	T name;
	T value;
};

}