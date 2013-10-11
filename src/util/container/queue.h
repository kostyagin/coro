#pragma once

#include <boost/move/move.hpp>
#include <boost/noncopyable.hpp>

#include <algorithm>
#include <utility>

#include <boost/system/error_code.hpp>

#include <functional>


namespace container {

// Queue с потреблением памяти как у вектора - столько, сколько было максимально по истории (потребление памяти только растёт)
template <class T> class Queue final: private boost::noncopyable {

public:
	explicit Queue(size_t capacity = 4);
	~Queue();

	size_t Size() const;
	bool Empty() const;
	void Push(const T& value);
	void Push(T&& value);
	template <class... Args> void Emplace(Args&&... args);
	const T& Top() const;
	T& Top();
	void Pop();

private:
	std::allocator<T> allocator;
	size_t capacity;
	T* a;
	size_t begin;
	size_t size;

	T* Allocate();
	void Deallocate();
	template <class... Args> void Add(Args&&... args);
	void DestroyFront();
	void ShiftBegin();
	void GrowIfFull();
	void Grow();
	void Swap(Queue<T>& another);
};

template <class T>
inline Queue<T>::Queue(size_t capacity)
	: capacity(capacity)
	, a{Allocate()}
	, begin{0}
	, size{0}
{
}

template <class T>
inline Queue<T>::~Queue() {
	try {
		while (!Empty()) {
			Pop();
		}
	} catch (...) {
		Deallocate();
		throw;
	}
	Deallocate();
}

template <class T>
inline size_t Queue<T>::Size() const {
	return size;
}

template <class T>
inline bool Queue<T>::Empty() const {
	return size == 0;
}

template <class T>
inline void Queue<T>::Push(const T& value) {
	Add(value);
}

template <class T>
inline void Queue<T>::Push(T&& value) {
	Add(std::forward<T>(value));
}

template <class T>
template <class... Args>
inline void Queue<T>::Emplace(Args&&... args) {
	Add(std::forward<Args>(args)...);
}

template <class T>
inline const T& Queue<T>::Top() const {
	return a[begin];
}

template <class T>
inline T& Queue<T>::Top() {
	return a[begin];
}

template <class T>
inline void Queue<T>::Pop() {
	--size;
	try {
		DestroyFront();
	} catch (...) {
		ShiftBegin();
		throw;	
	}
	ShiftBegin();
}

template <class T>
inline T* Queue<T>::Allocate() {
	return allocator.allocate(capacity, 0);
}

template <class T>
inline void Queue<T>::Deallocate()
{
	return allocator.deallocate(a, capacity);
}

template <class T>
template <class... Args>
void Queue<T>::Add(Args&&... args)
{
	GrowIfFull();
	new (a + (begin + size) % capacity) T(std::forward<Args>(args)...);
	++size;
}

template <class T>
inline void Queue<T>::DestroyFront()
{
	allocator.destroy(a + begin);
}

template <class T>
inline void Queue<T>::ShiftBegin()
{
	++begin;
	if (begin == capacity) {
		begin = 0;
	}
}

template <class T>
void Queue<T>::GrowIfFull()
{
	if (capacity == size) {
		Grow();
	}
}

namespace details {
template <class T>
T* Move(T* begin, T* end, T* dst) {
	while (begin < end) {
		new (dst++) T(std::move(*begin++));
	}
	return dst;
}
}

template <class T>
inline void Queue<T>::Grow() {
	Queue<T> another(capacity * 2);
	if (begin + size > capacity) {
		// два отрезка
		T* const out = details::Move(a + begin, a + capacity, another.a);
		details::Move(a, a + begin + size - capacity, out);
	} else {
		// один отрезок
		details::Move(a + begin, a + begin + size, another.a);
	}
	another.size = size;
	another.begin = 0;
	Swap(another);
};

template <class T>
inline void Queue<T>::Swap(Queue<T>& another)
{
	std::swap(capacity, another.capacity);
	std::swap(a, another.a);
	std::swap(begin, another.begin);
	std::swap(size, another.size);
}

}
