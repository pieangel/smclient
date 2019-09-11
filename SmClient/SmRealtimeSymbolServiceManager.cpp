#include "SmRealtimeSymbolServiceManager.h"
#include "SmWebsocketSession.h"
#include "SmSymbolManager.h"
#include "SmSymbol.h"
#include "SmMarketManager.h"
#include "SmHdClient.h"

void SmRealtimeSymbolServiceManager::RegisterAllRecentSymbol()
{
	SmHdClient* client = SmHdClient::GetInstance();
	SmMarketManager* mrktMgr = SmMarketManager::GetInstance();
	std::vector<SmSymbol*> symVec = mrktMgr->GetRecentMonthSymbolList();
	for (auto it = symVec.begin(); it != symVec.end(); ++it) {
		client->RegisterProduct((*it)->SymbolCode());
	}
}

void SmRealtimeSymbolServiceManager::RegisterSymbol(std::string user_id, std::string symCode)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = symMgr->FindSymbol(symCode);
	if (!sym)
		return;
	
}

void SmRealtimeSymbolServiceManager::RegisterSymbol(SmUser* user, SmSymbol* sym)
{
	if (!user || !sym)
		return;
	{
		std::lock_guard<std::mutex> lock(_mutex);

		auto it = _ServiceMap.find(sym);
		if (it != _ServiceMap.end()) {
			SmUserMap& userMap = it->second;
			userMap[user] = user;
		}
		else {
			SmUserMap userMap;
			userMap[user] = user;
			_ServiceMap[sym] = userMap;
		}
	}
}

void SmRealtimeSymbolServiceManager::UnregisterSymbol(std::string user_id, std::string symCode)
{
	
}

void SmRealtimeSymbolServiceManager::UnregisterSymbol(SmUser* user, SmSymbol* sym)
{
	if (!user || !sym)
		return;
	{
		std::lock_guard<std::mutex> lock(_mutex);

		auto it = _ServiceMap.find(sym);
		if (it != _ServiceMap.end()) {
			SmUserMap& userMap = it->second;
			auto itu = userMap.find(user);
			if (itu != userMap.end()) {
				userMap.erase(itu);
			}
		}
	}
}

void SmRealtimeSymbolServiceManager::UnregisterAllSymbol(std::string user_id)
{
	
}

void SmRealtimeSymbolServiceManager::UnregisterAllSymbol(SmUser* user)
{
	if (!user)
		return;
	{
		std::lock_guard<std::mutex> lock(_mutex);

		for (auto it = _ServiceMap.begin(); it != _ServiceMap.end(); ++it) {
			SmUserMap& userMap = it->second;
			auto itu = userMap.find(user);
			if (itu != userMap.end()) {
				userMap.erase(itu);
			}
		}
	}
}

void SmRealtimeSymbolServiceManager::Register(SmUser* user)
{
	
}

void SmRealtimeSymbolServiceManager::Unregister(std::string user_id)
{
	auto it = _UserMap.find(user_id);
	if (it != _UserMap.end()) {
		_UserMap.erase(it);
	}
}

void SmRealtimeSymbolServiceManager::SendInfo()
{
	
}

void SmRealtimeSymbolServiceManager::BroadcastSise()
{
	for (auto it = _ServiceMap.begin(); it != _ServiceMap.end(); ++it) {
		SendSise(it->first, it->second);
	}
}

void SmRealtimeSymbolServiceManager::BroadcastHoga()
{
	for (auto it = _ServiceMap.begin(); it != _ServiceMap.end(); ++it) {
		SendHoga(it->first, it->second);
	}
}

void SmRealtimeSymbolServiceManager::SendSise(SmSymbol* sym, SmUserMap& userMap)
{
	
}

void SmRealtimeSymbolServiceManager::SendHoga(SmSymbol* sym, SmUserMap& userMap)
{
	
}
