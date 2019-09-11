#pragma once
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "Global/TemplateSingleton.h"
class SmSymbol;
class SmUser;
class SmRealtimeSymbolServiceManager : public TemplateSingleton<SmRealtimeSymbolServiceManager>
{
private:
	std::mutex _mutex;
	SmSymbol* _Symbol = nullptr;
	std::map<std::string, SmUser*> _UserMap;
	typedef std::map<SmUser*, SmUser*> SmUserMap;
	std::map<SmSymbol*, SmUserMap> _ServiceMap;
public:
	void RegisterAllRecentSymbol();
	void RegisterSymbol(std::string user_id, std::string symCode);
	void UnregisterSymbol(std::string user_id, std::string symCode);
	void UnregisterAllSymbol(std::string user_id);


	void Register(SmUser* user);
	void Unregister(std::string user_id);
	void SendInfo();
	SmSymbol* Symbol() const { return _Symbol; }
	void Symbol(SmSymbol* val) { _Symbol = val; }
	void BroadcastSise();
	void BroadcastHoga();
private:
	void RegisterSymbol(SmUser* user, SmSymbol* sym);
	void UnregisterSymbol(SmUser* user, SmSymbol* sym);
	void UnregisterAllSymbol(SmUser* user);

	void SendSise(SmSymbol* sym, SmUserMap& userMap);
	void SendHoga(SmSymbol* sym, SmUserMap& userMap);
};

