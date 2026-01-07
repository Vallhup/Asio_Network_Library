#include "pch.h"
#include "SendBuffer.h"

SendBuffer* SendBufferPool::Acquire()
{
	if (_free.empty())
		return new SendBuffer;

	SendBuffer* buf = _free.back();
	_free.pop_back();
	return buf;
}

void SendBufferPool::Release(SendBuffer* buf)
{
	if (!buf) return;

	buf->size = 0;
	_free.push_back(buf);
}

SendBuffer* SendBuffer::Clone() const
{
	SendBuffer* copy = new SendBuffer;
	copy->size = size;
	
	std::memcpy(copy->data, data, size);
	return copy;
}
