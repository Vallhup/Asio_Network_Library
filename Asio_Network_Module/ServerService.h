#pragma once

#include "NetworkService.h"
#include "Acceptor.h"

class ServerService : public NetworkService {
public:
	explicit ServerService(uint16 threadCnt, uint16 port);
	virtual ~ServerService() = default;

protected:
	virtual void OnStart() override;
	virtual void OnStop() override;

private:
	Acceptor _acceptor;
};