#pragma once

#include <boost/container/vector.hpp>

#include <algorithm>
#include <stack>

namespace container {

template <class T> class Stack final {

public:
	explicit Stack(size_t capacity = 4);

	size_t Size() const;
	bool Empty() const;
	void Push(const T& value);
	void Push(T&& value);
	template <class... Args> void Emplace(Args&&... args);
	const T& Top() const;
	T& Top();
	void Pop();

private:
	std::stack<T, boost::container::vector<T>> impl;
};

namespace
{
template <class T>
boost::container::vector<T> PrepareVector(size_t capacity) {
	boost::container::vector<T> v;
	v.reserve(capacity);
	return v;
}
}

template <class T>
inline Stack<T>::Stack(size_t capacity)
	: impl(PrepareVector<T>(capacity))
{
}

template <class T>
inline size_t Stack<T>::Size() const {
	return impl.size();
}

template <class T>
inline bool Stack<T>::Empty() const {
	return impl.empty();
}

template <class T>
inline void Stack<T>::Push(const T& value) {
	impl.push(value);
}

template <class T>
inline void Stack<T>::Push(T&& value) {
	impl.push(std::forward<T>(value));
}

template <class T>
template <class... Args>
inline void Stack<T>::Emplace(Args&&... args) {
	impl.emplace(std::forward<Args>(args)...);
}

template <class T>
inline const T& Stack<T>::Top() const {
	return impl.top();
}

template <class T>
inline T& Stack<T>::Top() {
	return impl.top();
}

template <class T>
inline void Stack<T>::Pop() {
	impl.pop();
}

}
