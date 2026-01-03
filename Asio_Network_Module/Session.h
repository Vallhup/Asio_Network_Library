#pragma once

class ISessionListener;

class Session : public std::enable_shared_from_this<Session> {
public:
	Session() = delete;
	explicit Session(tcp::socket s, ISessionListener& listener);

	void Start();
	void Close();

	void Send(const void* data, uint64 size);

private:
	void Recv();
	void InternalSend();
	void ProcessPacket();

	tcp::socket _socket;
	asio::strand<asio::any_io_executor> _strand;

	ISessionListener& _listener;

	bool _closed;

	// TEMP : Use vector as a simple buffer
	std::array<uint8, 4096> _recvTemp;
	std::vector<uint8> _recvAccum;

	bool _isSending;
	std::deque<std::vector<uint8>> _sendQueue;
	std::vector<asio::const_buffer> _gatherBufs;
};

