#include "pch.h"
#include "SendBuffer.h"

SendBuffer* SendBufferPool::Acquire(uint32 capacity)
{
	const uint32 level = PickLevel(capacity);
	if (level != kInvalidLevel)
	{
		const uint32 cap = kCapacityList[level];

		SendBuffer* buf;
		if (_free[level].try_pop(buf))
		{
			buf->Reset();
			return buf;
		}

		return AllocateNew(cap);
	}

	return AllocateNew(capacity);
}

void SendBufferPool::Release(SendBuffer* buf)
{
	if (!buf) return;

	const uint32 level = LevelIndexByCapacity(buf->capacity);
	if (level == kInvalidLevel)
	{
		Destroy(buf);
		return;
	}

	buf->Reset();
	_free[level].push(buf);
}

uint32 SendBufferPool::PickLevel(uint32 capacity)
{
	for (uint32 i = 0; i < kCapacityList.size(); ++i)
	{
		if (capacity <= kCapacityList[i])
			return i;
	}

	return kInvalidLevel;
}

uint32 SendBufferPool::LevelIndexByCapacity(uint32 capacity)
{
	for (uint32 i = 0; i < kCapacityList.size(); ++i)
	{
		if (capacity == kCapacityList[i])
			return i;
	}

	return kInvalidLevel;
}

SendBuffer* SendBufferPool::AllocateNew(uint32 capacity)
{
	const uint64 totalSize = 
		sizeof(SendBuffer) + kCacheAlign + capacity;

	BYTE* rawByte = reinterpret_cast<BYTE*>(
		::operator new(totalSize, std::align_val_t(kCacheAlign)));

	BYTE* dataPtr = rawByte + sizeof(SendBuffer);
	uint64 dataSpace = totalSize - sizeof(SendBuffer);

	// aligned가 가리키는 주소부터 시작해서
	// 남은 공간 크기가 dataSpace인 메모리 범위 안에서
	// kCacheAlign 정렬을 만족하는 시작 주소를 찾아
	// 그 주소부터 capacity만큼의 바이트를 쓸 수 있으면
	// aligned를 그 주소로 갱신한다.
	// 
	// 못 찾으면 nullptr를 반환하고 aligned는 변경되지 않는다.
	void* aligned = dataPtr;
	if (!std::align(kCacheAlign, capacity, aligned, dataSpace))
	{
		::operator delete(rawByte, std::align_val_t(kCacheAlign));
		throw std::bad_alloc{};
	}

	SendBuffer* b = reinterpret_cast<SendBuffer*>(rawByte);
	b->size = 0;
	b->capacity = capacity;
	b->data = reinterpret_cast<BYTE*>(aligned);

	return b;
}

void SendBufferPool::Destroy(SendBuffer* buf)
{
	::operator delete(reinterpret_cast<void*>(buf),
		std::align_val_t(kCacheAlign));
}

SendBuffer* SendBuffer::Clone() const
{
	SendBuffer* copy = SendBufferPool::Get().Acquire(capacity);
	copy->size = size;
	
	std::memcpy(copy->data, data, size);
	return copy;
}

bool SendBuffer::TryAppend(const void* src, uint32 len)
{
	if (size + len > capacity) return false;

	std::memcpy(data + size, src, len);
	size += len;

	return true;
}

void SendBuffer::Reset()
{
	size = 0;
}