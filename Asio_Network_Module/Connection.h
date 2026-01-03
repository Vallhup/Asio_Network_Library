#pragma once

class IConnectionListener;

class Connection 
	: public std::enable_shared_from_this<Connection> {
	static constexpr size_t MAX_BUFFERS{ 32 };
	static constexpr size_t MAX_BYTES{ 64 * 1024 };

public:
	explicit Connection(tcp::socket s, IConnectionListener& listener);
	virtual ~Connection() = default;

	void Start();
	void Stop();

	void Send(std::span<const BYTE> data);

private:
	void DoRecv();
	void DoSend();

	void OnRecv(std::error_code ec, size_t bytes);
	void OnSend(std::error_code ec, size_t batchCount);

	void ProcessPacket();
	void Close();

	bool _isClosed;
	tcp::socket _socket;
	asio::strand<asio::any_io_executor> _strand;

	IConnectionListener& _listener;

	// TEMP : Use vector as a simple buffer
	std::array<BYTE, 4096> _recvTemp;
	std::vector<BYTE> _recvAccum;

	bool _isSending;
	std::deque<std::vector<BYTE>> _sendQueue;
	std::vector<asio::const_buffer> _gatherBufs;
};

