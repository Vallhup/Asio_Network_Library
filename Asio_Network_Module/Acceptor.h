#pragma once

class Acceptor {
public:
	Acceptor() = delete;
	Acceptor(asio::io_context& io, uint16 port);

	void Start();
	void Stop();

private:
	void Accept();
	void Close();

	bool _isRunning;
	tcp::acceptor _acceptor;
	asio::strand<asio::any_io_executor> _strand;
};

