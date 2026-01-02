#pragma once

#include "SessionManager.h"

class NetworkService {
public:
	NetworkService(uint16 threadCnt);
	virtual ~NetworkService() = default;

	void Start();
	void Stop();

protected:
	virtual void OnStart() = 0;
	virtual void OnStop() = 0;

private:
	void StartWorkers();
	void StopWorkers();

	uint16 _threadCnt;
	asio::io_context _ioCtx;
	std::optional<asio::executor_work_guard<asio::io_context::executor_type>> _workGuard;

	std::vector<std::thread> _workers;

	SessionManager _sessMng;
};

