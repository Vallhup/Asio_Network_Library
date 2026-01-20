#include "pch.h"
#include "ClientService.h"
#include "IConnectionListener.h"

ClientService::ClientService(uint16 threadCnt, std::string_view ip, uint16 port, 
    IConnectionListener& listener) : NetworkService(threadCnt), _ip(ip), _port(port),
    _listener(listener), _connection(nullptr)
{
}

void ClientService::Send(SendBuffer* data)
{
    _connection->Send(data);
}

void ClientService::OnStart()
{
    auto socket = std::make_shared<tcp::socket>(_ioCtx);
    tcp::endpoint endpoint{ asio::ip::make_address(_ip.c_str()), _port };

    socket->connect(endpoint);

    _connection = std::make_shared<Connection>(std::move(*socket), _listener);
    _connection->Start();
}

void ClientService::OnStop()
{
    if (_connection)
    {
        _connection->Stop();
        _connection.reset();
    }
}
