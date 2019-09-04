#include "SmRealtimeRegisterManager.h"
#include "SmUser.h"
#include "SmHdClient.h"
SmRealtimeRegisterManager::SmRealtimeRegisterManager()
{

}

SmRealtimeRegisterManager::~SmRealtimeRegisterManager()
{
	SmHdClient* client = SmHdClient::GetInstance();
	for (auto it = _RegisteredProduct.begin(); it != _RegisteredProduct.end(); ++it) {
		client->UnregisterProduct(*it);
	}
}

void SmRealtimeRegisterManager::RegisterProduct(std::string symCode)
{
	SmHdClient* client = SmHdClient::GetInstance();
	client->RegisterProduct(symCode);
	_RegisteredProduct.insert(symCode);
}
