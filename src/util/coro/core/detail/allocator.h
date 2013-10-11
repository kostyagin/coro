#pragma once

#include "src/util/check/check.h"

#include <boost/noncopyable.hpp>

namespace coro {
namespace detail {

class Allocator final {

public:
	Allocator();

	void* Allocate(size_t size);
	bool Deallocate(void*, size_t size);

private:
	static const size_t size = 1024;
	std::aligned_storage<size>::type storage;
	bool allocated;
};

inline Allocator::Allocator()
	: allocated(false)
{
}

inline void* Allocator::Allocate(size_t size) {
	Check(!allocated);
	if (size > this->size) {
		return 0;
	}
	allocated = true;
	return &storage;
}

inline bool Allocator::Deallocate(void* /*data*/, size_t size) {
	if (size > this->size) {
		return false;
	}
	allocated = false;
	return true;
}

}
}