#include "pch.h"
#include "SendBuffer.h"

SendBuffer* SendBufferPool::Acquire()
{
	SendBuffer* buf;
	if (_free.try_pop(buf))
	{
		return buf;
	}

	return new SendBuffer;
}

void SendBufferPool::Release(SendBuffer* buf)
{
	if (!buf) return;

	buf->size = 0;
	_free.push(buf);
}

SendBuffer* SendBuffer::Clone() const
{
	SendBuffer* copy = SendBufferPool::Get().Acquire();
	copy->size = size;
	
	std::memcpy(copy->data, data, size);
	return copy;
}
