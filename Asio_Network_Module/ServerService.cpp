#include "pch.h"
#include "ServerService.h"

ServerService::ServerService(uint16 threadCnt, uint16 port) 
	: NetworkService(threadCnt), _acceptor(_ioCtx, port)
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