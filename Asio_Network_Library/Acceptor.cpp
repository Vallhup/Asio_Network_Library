#include "pch.h"
#include "Acceptor.h"
#include "IAcceptListener.h"

Acceptor::Acceptor(asio::io_context& io, uint16 port, IAcceptListener& listener)
	: _acceptor(io, tcp::endpoint(tcp::v4(), port)), _listener(listener),
	_strand(io.get_executor()), _isRunning(false)
{
	_acceptor.set_option(asio::socket_base::reuse_address(true));
}

void Acceptor::Start()
{
	asio::dispatch(_strand,
		[this]()
		{
			if (!_isRunning)
			{
				_isRunning = true;
				DoAccept();
			}
		}
	);
}

void Acceptor::Stop()
{
	asio::dispatch(_strand,
		[this]()
		{
			if (_isRunning)
			{
				_isRunning = false;
				Close();
			}
		}
	);
}

void Acceptor::DoAccept()
{
	if (!_isRunning) return;

	_acceptor.async_accept(
		asio::bind_executor(_strand,
			[this](std::error_code ec, tcp::socket socket)
			{
				if (ec)
				{
					// Error Handling
				}

				else
				{
					_listener.OnAccept(std::move(socket));
				}

				if(_isRunning)
					DoAccept();
			}
		)
	);
}


void Acceptor::Close()
{
	std::error_code ec;
	_acceptor.close(ec);

	if (ec)
	{
		if (ec != asio::error::operation_aborted)
		{
			// Error Handling
		}
	}
}
