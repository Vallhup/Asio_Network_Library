#pragma once
#include "NetworkService.h"

class ClientService : public NetworkService {
public:
	ClientService(uint16 threadCnt);
	virtual ~ClientService() = default;

protected:
	virtual void OnStart() override;
	virtual void OnStop() override;
};