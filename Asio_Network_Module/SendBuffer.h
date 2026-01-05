#pragma once

struct SendBuffer {
	uint32 size;
	BYTE data[1024 * 1024];
};

class SendBufferPool {
public:
	SendBuffer* Acquire();
	void Release(SendBuffer* buf);

private:
	std::vector<SendBuffer*> _free;
};