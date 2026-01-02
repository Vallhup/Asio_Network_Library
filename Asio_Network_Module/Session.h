#pragma once

class Session {
public:
	Session();
	~Session();

private:
	tcp::socket _socket;
	asio::strand<asio::any_io_executor> _strand;
};

