#pragma once

#include "./slice.h"

#include <boost/range.hpp>

namespace boost {
	template<> struct range_mutable_iterator<::container::Slice> {
		using type = char*;
	};

	template<> struct range_const_iterator<::container::Slice> {
		using type = const char*;
	};

}

namespace container {

inline char* range_begin(container::Slice& slice) {
	return slice.begin;
}

inline char* range_end(container::Slice& slice) {
	return slice.end;
}

inline const char* range_begin(const container::Slice& slice) {
	return slice.begin;
}

inline const char* range_end(const container::Slice& slice) {
	return slice.end;
}

};