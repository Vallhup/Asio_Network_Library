#include "pch.h"
#include "Connection.h"
#include "IConnectionListener.h"
#include "Protocol.h"

/* --------------[ Connection ]--------------*/

Connection::Connection(tcp::socket s, IConnectionListener& listener
	, uint32 maxPacketSize) : _socket(std::move(s)), _strand(_socket.get_executor()), 
	_isClosed(false), _isSending(false), _listener(listener), 
	_maxPacketSize(maxPacketSize), _id(std::numeric_limits<uint32>::max())
{
}

void Connection::Start()
{
	auto self = shared_from_this();
	asio::dispatch(_strand,
		[this, self]()
		{
			_listener.OnConnected(*this);
			DoRecv();
		}
	);
}

void Connection::Stop()
{
	auto self = shared_from_this();
	asio::dispatch(_strand,
		[this, self]()
		{
			_listener.OnDisconnected(*this);
			Close();
		}
	);
}

void Connection::Send(std::span<const BYTE> data)
{
	if (data.size() == 0) return;

	SendBuffer* buffer = _sendPool.Acquire();
	buffer->size = static_cast<uint32>(data.size());
	memcpy(buffer->data, data.data(), buffer->size);

	auto self = shared_from_this();
	asio::dispatch(_strand,
		[this, self, buf = std::move(buffer)]()
		{
			if (_isClosed) return;

			_sendQueue.push_back(std::move(buf));
			if (!_isSending)
			{
				_isSending = true;
				DoSend();
			}
		});
}

void Connection::Send(SendBuffer* data)
{
	if (data->size == 0) return;

	auto self = shared_from_this();
	asio::dispatch(_strand,
		[this, self, buf = std::move(data)]()
		{
			if (_isClosed) return;

			_sendQueue.push_back(std::move(buf));
			if (!_isSending)
			{
				_isSending = true;
				DoSend();
			}
		});
}

void Connection::DoRecv()
{
	auto self = shared_from_this();
	_socket.async_read_some(
		asio::buffer(_recvTemp),
		asio::bind_executor(_strand,
			[self](std::error_code ec, size_t bytes)
			{
				self->OnRecv(ec, bytes);
			}
		)
	);
}

void Connection::DoSend()
{
	if (_sendQueue.empty()) return;

	_gatherBufs.clear();
	_gatherBufs.reserve(MAX_BUFFERS);

	size_t totalBytes{ 0 };
	size_t batchCount{ 0 };

	for (auto& data : _sendQueue)
	{
		if (batchCount >= MAX_BUFFERS) break;
		if (totalBytes + data->size > MAX_BYTES) break;

		_gatherBufs.emplace_back(asio::buffer(data->data, data->size));
		totalBytes += data->size;
		++batchCount;
	}

	auto self = shared_from_this();
	asio::async_write(
		_socket, _gatherBufs,
		asio::bind_executor(_strand,
			[this, self, batchCount](std::error_code ec, size_t)
			{
				OnSend(ec, batchCount);
			}
		)
	);
}

void Connection::OnRecv(std::error_code ec, size_t bytes)
{
	if (_isClosed) return;
	if (ec)
	{
		Stop();
		return;
	}

	_recvAccum.insert(_recvAccum.end(),
		_recvTemp.begin(), _recvTemp.begin() + bytes);

	ProcessPacket();
	DoRecv();
}

void Connection::OnSend(std::error_code ec, size_t batchCount)
{
	if (ec)
	{
		Stop();
		return;
	}

	for (size_t i = 0; i < batchCount; ++i)
	{
		SendBuffer* front = _sendQueue.front();
		_sendQueue.pop_front();
		_sendPool.Release(front);
	}
		

	if (!_sendQueue.empty())
		DoSend();

	else
	{
		_isSending = false;

		if (_isClosed)
			_sendQueue.clear();
	}
}

void Connection::ProcessPacket()
{
	while (true)
	{
		const uint32 receivedSize = static_cast<const uint32>(_recvAccum.size());
		if (receivedSize < sizeof(PacketHeader)) return;

		PacketHeader header;
		std::memcpy(&header, _recvAccum.data(), sizeof(PacketHeader));

		const uint32 packetSize = header.size;
		if (packetSize < sizeof(PacketHeader) || packetSize > _maxPacketSize)
		{
			Stop();
			return;
		}

		if (receivedSize < packetSize) return;

		std::vector<BYTE> packet(_recvAccum.begin(),
			_recvAccum.begin() + packetSize);

		_recvAccum.erase(_recvAccum.begin(),
			_recvAccum.begin() + packetSize);

		_listener.OnPacketReceived(*this, header, packet.data());
	}
}

void Connection::Close()
{
	if (_isClosed) return;
	_isClosed = true;

	if (!_isSending)
		_sendQueue.clear();

	std::error_code ec;
	_socket.cancel(ec);
	_socket.shutdown(tcp::socket::shutdown_both, ec);
	_socket.close(ec);

	if (ec)
	{
		if (ec != asio::error::operation_aborted)
		{
			// Error Handling
		}
	}
}