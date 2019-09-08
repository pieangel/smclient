#include "SmSessionManager.h"
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include "SmWebsocketSession.h"
#include "Json/json.hpp"
#include "SmErrorHandler.h"
#include "SmServiceDefine.h"
using namespace nlohmann;
SmSessionManager::SmSessionManager()
{
	int i = 0;
}

SmSessionManager::~SmSessionManager()
{
	if (_Session) {
		_Session->close();
	}
}

void SmSessionManager::Send(std::string msg)
{
	// Put the message in a shared pointer so we can re-use it for each client
	auto const ss = boost::make_shared<std::string const>(std::move(msg));
	if (_Session) {
		_Session->send(ss);
	}
}

void SmSessionManager::Login()
{
	if (!_Session)
		return;

	auto j3 = json::parse("{\"happy\": true, \"pi\": 3.141 }");

	std::string re = j3.dump();

	json loginInfo;
	loginInfo["req_id"] = (int)SmProtocol::req_login;
	loginInfo["user_info"]["id"] = _Session->Id();
	loginInfo["user_info"]["pwd"] = _Session->Pwd();
	std::string msg = loginInfo.dump(4);
	Send(loginInfo.dump(4));
}



void SmSessionManager::RegisterSiseSocket()
{
	if (!_Session)
		return;

	auto j3 = json::parse("{\"happy\": true, \"pi\": 3.141 }");

	std::string re = j3.dump();

	json send_info;
	send_info["req_id"] = (int)SmProtocol::req_register_sise_socket;
	send_info["user_info"]["id"] = _Session->Id();
	send_info["user_info"]["pwd"] = _Session->Pwd();
	std::string msg = send_info.dump(4);
	Send(send_info.dump(4));
}

void SmSessionManager::RegisterProduct(std::string symCode)
{
	if (!_Session)
		return;
	json reg_symbol;
	reg_symbol["req_id"] = (int)SmProtocol::req_register_symbol;
	reg_symbol["user_id"] = _Session->Id();
	reg_symbol["symbol_code"] = symCode;
	_RegSymbolSet.insert(symCode);
	Send(reg_symbol.dump(4));
}

void SmSessionManager::ReqestChartData(std::string symCode, int chartType, int cycle, int count)
{
	if (!_Session)
		return;
	json reg_symbol;
	reg_symbol["req_id"] = SmProtocol::req_chart_data;
	reg_symbol["user_id"] = _Session->Id();
	reg_symbol["symbol_code"] = symCode;
	reg_symbol["chart_type"] = std::to_string(chartType);
	reg_symbol["cycle"] = std::to_string(cycle);
	reg_symbol["count"] = std::to_string(count);
	Send(reg_symbol.dump(4));
}

void SmSessionManager::RegisterCycleData(std::string symCode, int chartType, int cycle, int count)
{
	if (!_Session)
		return;
	json reg_symbol;
	reg_symbol["req_id"] = SmProtocol::req_register_chart_cycle_data;
	reg_symbol["user_id"] = _Session->Id();
	reg_symbol["symbol_code"] = symCode;
	reg_symbol["chart_type"] = std::to_string(chartType);
	reg_symbol["cycle"] = std::to_string(cycle);
	reg_symbol["count"] = std::to_string(4);
	Send(reg_symbol.dump(4));
}

void SmSessionManager::RequestSiseData(std::string symCode)
{
	if (!_Session)
		return;
	json reg_symbol;
	reg_symbol["req_id"] = SmProtocol::req_sise_data;
	reg_symbol["user_id"] = _Session->Id();
	reg_symbol["symbol_code"] = symCode;
	Send(reg_symbol.dump(4));
}

void SmSessionManager::OnMessage(std::string message)
{
	SmErrorHandler* erHdlr = SmErrorHandler::GetInstance();
	erHdlr->ShowMessage(message);
}

void SmSessionManager::Close()
{
	if (_Session) {
		_Session->close();
	}
}

void SmSessionManager::UnregisterAllSymbol()
{
	if (!_Session)
		return;
	for (auto it = _RegSymbolSet.begin(); it != _RegSymbolSet.end(); ++it) {
		std::string symCode = *it;
		json reg_symbol;
		reg_symbol["req_id"] = 2;
		reg_symbol["user_id"] = _Session->Id();
		reg_symbol["symbol_code"] = symCode;
		Send(reg_symbol.dump(4));
	}
}
