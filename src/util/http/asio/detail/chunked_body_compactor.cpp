#include "./chunked_body_compactor.h"

#include "src/util/check/check.h"
#include "src/util/container/buffer.h"

http::asio::detail::ChunkedBodyCompactor::ChunkedBodyCompactor(container::Buffer& buffer, size_t headersLength)
	: buffer{buffer}
	, usedSpace{headersLength}
	, state{State::ReadLength}
	, lengthStart{headersLength}
{
	AddChunk(headersLength);
}

bool http::asio::detail::ChunkedBodyCompactor::IsFinished() const {
	return state == State::Finished;
}

size_t http::asio::detail::ChunkedBodyCompactor::GetUsedSpace() const {
	return usedSpace;
}

void http::asio::detail::ChunkedBodyCompactor::AddChunk(size_t begin) {
	while (begin != buffer.GetSize()) {
		switch (state) {
			case State::ReadLength:
				AddChunkInReadLengthState(begin);
				break;
			case State::ReadData:
				AddChunkInReadDataState(begin);
				break;
			case State::Skip:
				AddChunkInSkipState(begin);
				break;
			case State::Finished:
			default:
				CheckFalse();
		}
	}
}

void http::asio::detail::ChunkedBodyCompactor::AddChunkInReadLengthState(size_t& begin) {
	const size_t end = buffer.GetSize();
	const char* const from = buffer.GetPlace() + begin;
	const char* const to = buffer.GetPlace() + end;
	const char* const eol = std::find(from, to, '\n');
	if (eol == to) {
		const size_t toCopy = to - from;
		std::memmove(buffer.GetPlace() + usedSpace, from, toCopy);
		usedSpace += toCopy;
		begin = end;
	} else {
		const char* whereLength;
		if (usedSpace == lengthStart) {
			whereLength = from;
		} else {
			std::memmove(buffer.GetPlace() + usedSpace, from, eol - from);
			whereLength = buffer.GetPlace() + lengthStart;
		}
		begin += eol - from + 1;
		usedSpace = lengthStart;
		state = State::ReadData;
		toReadBytes = strtoul(whereLength, nullptr, 16);
		lastLengthIsZero = toReadBytes == 0;					
	}
}

void http::asio::detail::ChunkedBodyCompactor::AddChunkInReadDataState(size_t& begin) {
	const size_t end = buffer.GetSize();
	if (end - begin < toReadBytes) {
		const size_t toCopy = end - begin;
		std::memmove(buffer.GetPlace() + usedSpace, buffer.GetPlace() + begin, toCopy);
		usedSpace += toCopy;
		toReadBytes -= toCopy;
		begin = end;
	} else {
		std::memmove(buffer.GetPlace() + usedSpace, buffer.GetPlace() + begin, toReadBytes);
		begin += toReadBytes;
		usedSpace += toReadBytes;
		state = State::Skip;
		toSkipBytes = 2;
	}
}

void http::asio::detail::ChunkedBodyCompactor::AddChunkInSkipState(size_t& begin) {
	const size_t end = buffer.GetSize();
	const size_t chunkLength = end - begin;
	if (chunkLength < toSkipBytes) {
		toSkipBytes -= chunkLength;
		begin = end;
	} else {
		begin += toSkipBytes;
		if (lastLengthIsZero) {
			state = State::Finished;
		} else {
			state = State::ReadLength;
			lengthStart = usedSpace;
		}
	}
}
