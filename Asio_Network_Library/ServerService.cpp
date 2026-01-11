#include "pch.h"
#include "ServerService.h"

ServerService::ServerService(uint16 threadCnt, uint16 port, 
	IConnectionListener& connListener) : NetworkService(threadCnt), 
	_acceptor(_ioCtx, port, *this), _connListener(connListener)
{
}

void ServerService::OnStart()
{
	_acceptor.Start();
}

void ServerService::OnStop()
{
	_acceptor.Stop();
}