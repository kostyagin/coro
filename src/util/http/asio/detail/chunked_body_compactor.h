#pragma once

#include <boost/noncopyable.hpp>

namespace container {
class Buffer;
};

namespace http {
namespace asio {
namespace detail {

class ChunkedBodyCompactor final: private boost::noncopyable {

public:
	ChunkedBodyCompactor(container::Buffer& buffer, size_t headersLength);

	// Закончено ли чтение
	bool IsFinished() const;
	// Сколько места используют заголовки и компактифицированное тело
	size_t GetUsedSpace() const;
	// Добавить новый чанк (начиная с begin до конца буфера)
	void AddChunk(size_t begin);

private:
	enum class State {
		ReadLength,
		ReadData,
		Skip,
		Finished
	};
	container::Buffer& buffer;
	size_t usedSpace;
	State state;
	union {
		size_t lengthStart; // State::ReadLength
		size_t toReadBytes; // State::ReadData
		size_t toSkipBytes; // State::Skip
	};
	bool lastLengthIsZero;

	void AddChunkInReadLengthState(size_t& begin);
	void AddChunkInReadDataState(size_t& begin);
	void AddChunkInSkipState(size_t& begin);
};

}
}
}
