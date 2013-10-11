#pragma once

#include <memory>

namespace container {

class Buffer final {

public:
	explicit Buffer(size_t initialCapacity = sizeof(size_t));
	~Buffer();

	void Clear();
	void ReserveForAppend(size_t count);
	Buffer& Append(char c);
	Buffer& Append(const void* from, size_t count);
	template <size_t N>	Buffer& Append(const char (&data)[N]);
	template <class String> Buffer& Append(const String& str);

	const char* GetPlace() const;
	char* GetPlace();
	size_t GetSize() const;
	size_t GetCapacity() const;

	void Resize(size_t size);

private:
	static_assert(sizeof(char) == 1, "non-byte char");
	std::allocator<char> allocator;
	char* place;
	size_t capacity;
	size_t size;

	void Grow(size_t toReserve);
};

inline Buffer::Buffer(size_t initialCapacity)
	: place{allocator.allocate(initialCapacity)}
	, capacity{initialCapacity}
	, size{0}
{
}

inline Buffer::~Buffer() {
	allocator.deallocate(place, capacity);
}

inline void Buffer::Clear() {
	size = 0;
}

inline void Buffer::ReserveForAppend(size_t count)
{
	const size_t toReserve = size + count;
	if (toReserve >= capacity) {
		Grow(toReserve);
	}
}

inline Buffer& Buffer::Append(char c) {
	ReserveForAppend(sizeof(c));
	*(place + size) = c;
	++size;
	return *this;
}

inline Buffer& Buffer::Append(const void* from, size_t count) {
	ReserveForAppend(count);
	std::memcpy(place + size, from, count);
	size += count;
	return *this;
}

template <size_t N>
inline Buffer& Buffer::Append(const char (&data)[N]) {
	return Append(data, N - 1);
}

template <class String>
inline Buffer& Buffer::Append(const String& str) {
	return Append(str.c_str(), str.size());
}

inline const char* Buffer::GetPlace() const {
	return place;
}

inline char* Buffer::GetPlace() {
	return place;
}

inline size_t Buffer::GetSize() const {
	return size;
}

inline size_t Buffer::GetCapacity() const {
	return capacity;
}

inline void Buffer::Resize(size_t size) {
	if (size >= capacity){
		this->size = capacity;
		Grow(size);
	}
	this->size = size;
}

inline void Buffer::Grow(size_t toReserve) {
	size_t newCapacity = capacity;
	do {
		newCapacity *= 2;
	} while (newCapacity <= toReserve);
	Buffer buffer(newCapacity);
	std::memcpy(buffer.place, place, size);
	std::swap(buffer.place, place);
	buffer.capacity = capacity;
	capacity = newCapacity;
}

}