#include "pch.h"
#include "NetworkService.h"

NetworkService::NetworkService(uint16 threadCnt)
	: _ioCtx(), _threadCnt(threadCnt)
{
}

void NetworkService::Start()
{
	if (!_workGuard.has_value())
	{
		_workGuard.emplace(asio::make_work_guard(_ioCtx));
		OnStart();
		StartWorkers();
	}
}

void NetworkService::Stop()
{
	OnStop();
	StopWorkers();
}

void NetworkService::StartWorkers()
{
	for (uint16 i = 0; i < _threadCnt; ++i)
		_workers.emplace_back([this]() { _ioCtx.run(); });
}

void NetworkService::StopWorkers()
{
	_workGuard.reset();

	for (auto& worker : _workers)
	{
		if(worker.joinable())
			worker.join();
	}
	_workers.clear();
}
