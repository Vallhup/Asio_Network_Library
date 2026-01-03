#include "pch.h"
#include "Session.h"
#include "ISessionListener.h"

Session::Session(tcp::socket s, ISessionListener& listener)
	: _socket(std::move(s)), _strand(_socket.get_executor()), 
	_listener(listener), _closed(false), _isSending(false)
{
}

void Session::Start()
{
	auto self = shared_from_this();
	asio::dispatch(_strand,
		[this, self]()
		{
			_listener.OnConnected();
			Recv();
		});
}

void Session::Close()
{
	auto self = shared_from_this();
	asio::dispatch(_strand,
		[this, self]()
		{
			if (_closed) return;
			_closed = true;

			std::error_code ec;
			_socket.shutdown(tcp::socket::shutdown_both, ec);
			_socket.close(ec);

			if (!_isSending)
				_sendQueue.clear();

			_listener.OnDisconnected();

			if (ec)
				std::cerr << "Socket close error: " << ec.message() << std::endl;
		});
}

void Session::Send(const void* data, uint64 size)
{
	if (size == 0) return;

	std::vector<uint8> buffer(size);
	std::memcpy(buffer.data(), data, size);

	auto self = shared_from_this();
	asio::dispatch(_strand,
		[this, self, buffer = std::move(buffer)]()
		{
			if (_closed) return;

			_sendQueue.emplace_back(std::move(buffer));

			if (!_isSending)
			{
				_isSending = true;
				InternalSend();
			}
		});
}

void Session::Recv()
{
	auto self = shared_from_this();
	_socket.async_read_some(
		asio::buffer(_recvTemp),
		asio::bind_executor(_strand,
			[this, self](std::error_code ec, size_t bytes)
			{
				if (ec)
				{
					if (ec != asio::error::eof and
						ec != asio::error::operation_aborted)
					{
						std::cout << "Receive Error on Session[" << 0 << "] EC["
							<< ec.message() << "]\n";
					}

					Close();
					return;
				}

				_recvAccum.insert(_recvAccum.end(),
					_recvTemp.begin(), _recvTemp.begin() + bytes);

				ProcessPacket();
				Recv();
			})
	);
}

void Session::InternalSend()
{
	if (_sendQueue.empty()) return;

	_gatherBufs.clear();
	_gatherBufs.reserve(32);

	uint64 totalBytes{ 0 };
	uint64 batchCount{ 0 };

	for (auto& data : _sendQueue)
	{
		if (batchCount >= 32) break;
		if (totalBytes + data.size() > (64 * 1024)) break;

		_gatherBufs.emplace_back(asio::buffer(data.data(), data.size()));
		totalBytes += data.size();
		++batchCount;
	}

	auto self = shared_from_this();
	asio::async_write(
		_socket, _gatherBufs,
		asio::bind_executor(_strand,
			[this, self, batchCount](std::error_code ec, size_t)
			{
				if (ec)
				{
					if (ec != asio::error::eof and
						ec != asio::error::operation_aborted)
					{
						std::cout << "Send Error on Session[" << 0 << "] EC["
							<< ec.message() << "]\n";
					}

					Close();
					return;
				}

				for (uint64 i = 0; i < batchCount; ++i)
				{
					_sendQueue.pop_front();
				}

				if (!_sendQueue.empty())
					InternalSend();

				else
					_isSending = false;
			})
	);
}

void Session::ProcessPacket()
{
	while (true)
	{
		// TODO : Implement packet parsing logic here
		//_listener.OnDataReceived(_recvAccum.data(), 1);
		break;
	}
}
