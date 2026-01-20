#include "pch.h"
#include "ConnectionRegistry.h"

void ConnectionRegistry::Add(const std::shared_ptr<Connection> c)
{
	_connections.insert(std::make_pair(c->GetId(), c));
}

void ConnectionRegistry::Remove(uint32 id)
{
	auto it = _connections.find(id);
	if (it != _connections.end())
		it->second.store(nullptr);
}

std::shared_ptr<Connection> ConnectionRegistry::GetConnection(uint32 id)
{
	auto it = _connections.find(id);
	if (it != _connections.end()) return it->second.load();
	return nullptr;
}

void ConnectionRegistry::Send(uint32 id, SendBuffer* data)
{
	if (auto conn = GetConnection(id))
		conn->Send(data);
}

void ConnectionRegistry::Broadcast(SendBuffer* data, uint32 expected)
{
	for (auto& [id, conn] : _connections)
	{
		if (id == expected) continue;
		if (auto connPtr = conn.load())
		{
			SendBuffer* copy = data->Clone();
			connPtr->Send(copy);
		}
	}
}
