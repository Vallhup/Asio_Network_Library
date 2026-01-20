#include "pch.h"
#include "StressTestClientService.h"

StressTestClientService::StressTestClientService(uint16 threadCnt, 
	std::string_view ip, uint16 port, IConnectionListener& listener) 
	: NetworkService(threadCnt), _ip(ip), _port(port), 
	_listener(listener)
{
}

void StressTestClientService::Send(uint32 id, SendBuffer* data)
{
	_connRegistry.Send(id, data);
}

void StressTestClientService::Broadcast(SendBuffer* data, uint32 expected)
{
	_connRegistry.Broadcast(data, expected);
}