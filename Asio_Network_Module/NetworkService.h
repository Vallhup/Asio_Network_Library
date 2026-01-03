#pragma once

#include "SessionManager.h"

class NetworkService {
public:
	virtual ~NetworkService() = default;

	void Start();
	void Stop();

protected:
	NetworkService(uint16 threadCnt);

	virtual void OnStart() = 0;
	virtual void OnStop() = 0;

	asio::io_context _ioCtx;
	SessionManager _sessMng;

private:
	void StartWorkers();
	void StopWorkers();

	uint16 _threadCnt;
	std::vector<std::thread> _workers;
	std::optional<asio::executor_work_guard<asio::io_context::executor_type>> _workGuard;
};

