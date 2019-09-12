#pragma once
#include "Global/TemplateSingleton.h"
#include <string>
#include <set>
class SmWebsocketSession;
class SmSymbol;
class SmSessionManager : public TemplateSingleton<SmSessionManager>
{
public:
	SmSessionManager();
	~SmSessionManager();
	void Send(std::string msg);
	SmWebsocketSession* Session() const { return _Session; }
	void Session(SmWebsocketSession* val) { _Session = val; }
	void Login();
	void RegisterSiseSocket();
	void RegisterProduct(std::string symCode);
	void ReqestChartData(std::string symCode, int chartType, int cycle, int count);
	void RegisterCycleData(std::string symCode, int chartType, int cycle, int count);
	void RequestSiseData(std::string symCode);
	void OnMessage(std::string message);
	void Close();
	void SendReqUpdateQuote(std::string symbol_code);
	void SendReqUpdateHoga(std::string symbol_code);
	void SendReqUpdateQuote(SmSymbol* symbol);
	void SendReqUpdateHoga(SmSymbol* symbol);
private:
	SmWebsocketSession* _Session = nullptr;
	std::set<std::string> _RegSymbolSet;
	void UnregisterAllSymbol();
};

