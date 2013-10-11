#pragma once

namespace container {

struct Slice final {
	char* begin;
	char* end;

	Slice(char* begin = 0, char* end = 0);
};

inline Slice::Slice(char* begin, char* end)
	: begin{begin}
	, end{end}
{
}

}