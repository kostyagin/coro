#pragma once

#include "./debug_new.h"

#include <atomic>
#include <exception>
#include <cstdlib>

#ifdef NDEBUG
uint64_t debug_new::GetAllocationsCount() {
	return 0;
}
#else

namespace {

std::atomic_uint64_t allocationsCount;
const auto countMemoryOrder = std::memory_order_relaxed;

inline void* Allocate(size_t s) {
	allocationsCount.fetch_add(1, countMemoryOrder);
	void* m = malloc(s);
	if (m) {
		return m;
	}
	throw std::bad_alloc();
}

inline void Deallocate(void* p) {
	free(p);
}

}

uint64_t debug_new::GetAllocationsCount() {
	return allocationsCount.load(countMemoryOrder);
}

void* operator new(size_t s) {
	return Allocate(s);
}

void* operator new[](size_t s) {
	return Allocate(s);
}

void operator delete(void* p) {
	Deallocate(p);
}

void operator delete[](void* p) {
	Deallocate(p);
} 

#endif
