#pragma once

class Connection;

class IConnectionListener {
public:
	virtual ~IConnectionListener() = default;

	virtual void OnConnected(Connection& owner) = 0;
	virtual void OnDisconnected(Connection& owner) = 0;
	virtual void OnPacketReceived(Connection& owner, std::span<const BYTE> data) = 0;
};