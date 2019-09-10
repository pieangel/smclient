#include "pch.h"
#include "SmProtocolManager.h"
//#include "SmUserManager.h"
//#include "Json/json.hpp"
//#include "SmRealtimeSymbolServiceManager.h"
#include "SmSessionManager.h"
#include "SmServiceDefine.h"
//#include "SmOrderDefine.h"
//#include "SmTotalOrderManager.h"
#include "SmChartDefine.h"
#include "SmTimeSeriesCollector.h"
#include "SmTimeSeriesServiceManager.h"
#include "SmSymbolManager.h"
#include "SmSymbol.h"
#include "SmMarketManager.h"
#include "SmChartDataManager.h"
#include "SmChartData.h"
#include "SmMarketManager.h"
#include "SmHdClient.h"
using namespace nlohmann;
SmProtocolManager::SmProtocolManager()
{

}

SmProtocolManager::~SmProtocolManager()
{

}

void SmProtocolManager::OnMessage(std::string message, SmWebsocketSession* socket)
{
	if (!socket)
		return;

	CString msg;
	msg.Format(_T("message = %s, %x\n"), message.c_str(), socket);
	TRACE(msg);
	ParseMessage(message, socket);
}

void SmProtocolManager::ParseMessage(std::string message, SmWebsocketSession* socket)
{
	if (!socket) return;
	if (socket && message.length() == 0) {
		
		return;
	}

	try {
		auto json_object = json::parse(message);

		int req_id = json_object["req_id"];
		SmProtocol sm_protocol = [](int id) {
			return (SmProtocol)id;
		}(req_id);

		switch (sm_protocol)
		{
		case SmProtocol::req_login:
			OnLogin(json_object, socket);
			break;
		case  SmProtocol::req_register_symbol:
			OnRegisterSymbol(json_object);
			break;
		case SmProtocol::req_register_chart_cycle_data:
			OnRegisterChartCycleData(json_object);
			break;
		case SmProtocol::req_order_new:
		case SmProtocol::req_order_modify:
		case SmProtocol::req_order_cancel:
			OnOrder(json_object);
			break;
		case SmProtocol::req_chart_data:
			OnReqChartData(json_object);
			break;
		case SmProtocol::req_sise_data:
			OnReqSiseData(json_object);
			break;
		case SmProtocol::req_hoga_data:
			OnReqHogaData(json_object);
			break;
		case SmProtocol::req_symbol_master:
			OnReqSymbolMaster(json_object);
			break;
		case SmProtocol::req_symbol_master_all:
			OnReqSymbolMasterAll(json_object);
			break;
		case SmProtocol::req_recent_sise_data_all:
			OnReqRecentSiseDataAll(json_object);
			break;
		case SmProtocol::req_sise_data_all:
			OnReqSiseDataAll(json_object);
			break;
		case SmProtocol::req_register_recent_realtime_sise_all:
			OnReqRegisterRecentRealtimeSiseAll(json_object);
			break;
		case SmProtocol::req_market_list:
			OnReqMarketList(json_object);
			break;
		case SmProtocol::req_symbol_list_by_category:
			OnReqSymbolListByCategory(json_object);
			break;
		case SmProtocol::req_chart_data_from_main_server:
			OnReqChartDataFromMainServer(json_object);
			break;
		default:
			break;
		}
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}


void SmProtocolManager::OnLogin(nlohmann::json& obj, SmWebsocketSession* socket)
{
	try {
		auto user_info = obj["user_info"];
		std::string id = user_info["id"];
		std::string pwd = user_info["pwd"];
		//SmUserManager* userMgr = SmUserManager::GetInstance();
		//userMgr->OnLogin(id, pwd, socket);
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnLogout(nlohmann::json& obj)
{
	try {
		auto user_info = obj["user_info"];
		std::string id = user_info["id"];
		//SmUserManager* userMgr = SmUserManager::GetInstance();
		//userMgr->OnLogout(id);
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::SendResult(std::string user_id, int result_code, std::string result_msg)
{
	json res = {
		{"result_code", result_code},
		{"result_msg", result_msg}
	};
	//SmUserManager* userMgr = SmUserManager::GetInstance();
	//userMgr->SendResultMessage(user_id, res.dump(4));
}


void SmProtocolManager::SendResult(std::string user_id, SmProtocol protocol, int result_code, std::string result_msg)
{
	json res = {
		{"res_id", (int)protocol},
		{"result_code", result_code},
		{"result_msg", result_msg}
	};
	//SmUserManager* userMgr = SmUserManager::GetInstance();
	//userMgr->SendResultMessage(user_id, res.dump(4));
}

void SmProtocolManager::OnRegisterSymbol(nlohmann::json& obj)
{
	try {
		std::string id = obj["user_id"];
		std::string symCode = obj["symbol_code"];
		//SmRealtimeSymbolServiceManager* rtlSymMgr = SmRealtimeSymbolServiceManager::GetInstance();
		//rtlSymMgr->RegisterSymbol(id, symCode);

		SendResult(id, SmProtocol::res_register_symbol, 0, "register symbol success!");
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnUnregisterSymbol(nlohmann::json& obj)
{
	try {
		std::string id = obj["user_id"];
		std::string symCode = obj["symbol_code"];
		//SmRealtimeSymbolServiceManager* rtlSymMgr = SmRealtimeSymbolServiceManager::GetInstance();
		//rtlSymMgr->UnregisterSymbol(id, symCode);

		SendResult(id, SmProtocol::res_unregister_symbol, 0, "register symbol success!");
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnRegisterChartCycleData(nlohmann::json& obj)
{
	try {
		std::string id = obj["user_id"];
		std::string symCode = obj["symbol_code"];
		std::string chart_type = obj["chart_type"];
		std::string cycle = obj["cycle"];
		std::string count = obj["count"];
		SmChartDataRequest req;
		req.reqType = SmChartDataReqestType::CYCLE;
		req.user_id = id;
		req.symbolCode = symCode;
		req.chartType = (SmChartType)std::stoi(chart_type);
		req.cycle = std::stoi(cycle);
		req.count = std::stoi(count);
		req.next = 0;
		SmTimeSeriesServiceManager* timeSvcMgr = SmTimeSeriesServiceManager::GetInstance();
		timeSvcMgr->OnRegisterCycleDataRequest(std::move(req));
		SendResult(id, SmProtocol::res_register_chart_cycle_data,  0, "Success!");
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnUnregisterChartCycleData(nlohmann::json& obj)
{
	try {
		std::string id = obj["id"];
		std::string symCode = obj["symbol_code"];
		std::string chart_type = obj["chart_type"];
		std::string cycle = obj["cycle"];
		SendResult(id, SmProtocol::res_unregister_chart_cycle_data, 0, "success!");
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnOrder(nlohmann::json& obj)
{

}

void SmProtocolManager::OnReqChartData(nlohmann::json& obj)
{
	try {
		std::string chart_id = obj["chart_id"];
		std::string id = obj["user_id"];
		std::string symCode = obj["symbol_code"];
		int chart_type = obj["chart_type"];
		int cycle = obj["cycle"];
		int count = obj["count"];
		SmChartDataRequest req;
		req.reqType = SmChartDataReqestType::FIRST;
		req.user_id = id;
		req.symbolCode = symCode;
		req.chartType = (SmChartType)chart_type;
		req.cycle = cycle;
		req.count = count;
		req.next = 0;
		SmTimeSeriesServiceManager* timeSvcMgr = SmTimeSeriesServiceManager::GetInstance();
		timeSvcMgr->OnChartDataRequest(std::move(req));
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqSiseData(nlohmann::json& obj)
{
	try {
		std::string id = obj["user_id"];
		std::string symCode = obj["symbol_code"];
		SmSiseDataRequest req;
		req.symbol_code = symCode;
		req.user_id = id;
		SmTimeSeriesServiceManager* timeSvcMgr = SmTimeSeriesServiceManager::GetInstance();
		timeSvcMgr->OnSiseDataRequest(std::move(req));
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqHogaData(nlohmann::json& obj)
{
	try {
		std::string id = obj["user_id"];
		std::string symCode = obj["symbol_code"];
		SmHogaDataRequest req;
		req.symbol_code = symCode;
		req.user_id = id;
		SmTimeSeriesServiceManager* timeSvcMgr = SmTimeSeriesServiceManager::GetInstance();
		timeSvcMgr->OnHogaDataRequest(std::move(req));
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqSymbolMaster(nlohmann::json& obj)
{
	try {
		std::string id = obj["user_id"];
		std::string symCode = obj["symbol_code"];
		SmSymbolMasterRequest req;
		req.symbol_code = symCode;
		req.user_id = id;
		SmTimeSeriesServiceManager* timeSvcMgr = SmTimeSeriesServiceManager::GetInstance();
		timeSvcMgr->OnSymbolMasterRequest(std::move(req));
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqSymbolMasterAll(nlohmann::json& obj)
{
	try {
		SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
		std::map<std::string, SmSymbol*>& symbolMap = symMgr->GetSymbolMap();
		for (auto it = symbolMap.begin(); it != symbolMap.end(); ++it) {
			std::string id = obj["user_id"];
			std::string symCode = it->second->SymbolCode();
			SmSymbolMasterRequest req;
			req.symbol_code = symCode;
			req.user_id = id;
			SmTimeSeriesServiceManager* timeSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			timeSvcMgr->OnSymbolMasterRequest(std::move(req));
		}
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqSiseDataAll(nlohmann::json& obj)
{
	try {
		SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
		std::map<std::string, SmSymbol*>& symbolMap = symMgr->GetSymbolMap();
		for (auto it = symbolMap.begin(); it != symbolMap.end(); ++it) {
			std::string id = obj["user_id"];
			std::string symCode = it->second->SymbolCode();
			SmSiseDataRequest req;
			req.symbol_code = symCode;
			req.user_id = id;
			SmTimeSeriesServiceManager* timeSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			timeSvcMgr->OnSiseDataRequest(std::move(req));
		}
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqRecentSiseDataAll(nlohmann::json& obj)
{
	try {
		SmMarketManager* symMgr = SmMarketManager::GetInstance();
		std::vector<SmSymbol*> symVec = symMgr->GetRecentMonthSymbolList();
		for (auto it = symVec.begin(); it != symVec.end(); ++it) {
			std::string id = obj["user_id"];
			std::string symCode = (*it)->SymbolCode();
			SmSiseDataRequest req;
			req.symbol_code = symCode;
			req.user_id = id;
			SmTimeSeriesServiceManager* timeSvcMgr = SmTimeSeriesServiceManager::GetInstance();
			timeSvcMgr->OnSiseDataRequest(std::move(req));
		}
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqRegisterRecentRealtimeSiseAll(nlohmann::json& obj)
{
	try {
		SmMarketManager* symMgr = SmMarketManager::GetInstance();
		std::vector<SmSymbol*> symVec = symMgr->GetRecentMonthSymbolList();
		for (auto it = symVec.begin(); it != symVec.end(); ++it) {
			std::string id = obj["user_id"];
			std::string symCode = (*it)->SymbolCode();
			//SmRealtimeSymbolServiceManager* rtlSymMgr = SmRealtimeSymbolServiceManager::GetInstance();
			//rtlSymMgr->RegisterSymbol(id, symCode);
		}
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqMarketList(nlohmann::json& obj)
{
	try {
		SmMarketManager* marketMgr = SmMarketManager::GetInstance();
		std::string id = obj["user_id"];
		marketMgr->SendMarketList(id);
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqSymbolListByCategory(nlohmann::json& obj)
{
	try {
		SmMarketManager* marketMgr = SmMarketManager::GetInstance();
		std::string id = obj["user_id"];
		marketMgr->SendSymbolListByCategory(id);
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}

void SmProtocolManager::OnReqChartDataFromMainServer(nlohmann::json& obj)
{
	try {
		int session_id = obj["session_id"];
		std::string user_id = obj["user_id"];
		int service_req_id = obj["service_req_id"];
		std::string symCode = obj["symbol_code"];
		int chart_type = obj["chart_type"];
		int cycle = obj["cycle"];
		int count = obj["count"];
		SmChartDataRequest req;
		req.service_req_id = service_req_id;
		req.session_id = session_id;
		req.user_id = user_id;
		req.reqType = SmChartDataReqestType::FIRST;
		req.symbolCode = symCode;
		req.chartType = (SmChartType)chart_type;
		req.cycle = cycle;
		req.count = count;
		req.next = 0;
		SmHdClient* client = SmHdClient::GetInstance();
		client->GetChartData(req);
	}
	catch (std::exception e) {
		std::string error = e.what();
	}
}
