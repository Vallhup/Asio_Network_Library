#pragma once

class ISessionListener {
public:
	virtual ~ISessionListener() = default;

	virtual void OnConnected() = 0;
	virtual void OnDisconnected() = 0;
	virtual void OnDataReceived(const void* data, uint64 size) = 0;
};

