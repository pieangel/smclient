#pragma once
#include "Global/TemplateSingleton.h"
#include <string>
#include <set>
class SmSession;
class SmSessionManager : public TemplateSingleton<SmSessionManager>
{
public:
	SmSessionManager();
	~SmSessionManager();
	void Send(std::string msg);
	SmSession* Session() const { return _Session; }
	void Session(SmSession* val) { _Session = val; }
	void Login();
	void RegisterProduct(std::string symCode);
	void ReqestChartData(std::string symCode, int chartType, int cycle, int count);
	void OnMessage(std::string message);
	void Close();
private:
	SmSession* _Session = nullptr;
	std::set<std::string> _RegSymbolSet;
	void UnregisterAllSymbol();
};

