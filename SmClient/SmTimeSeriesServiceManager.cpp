#include "pch.h"
#include "SmTimeSeriesServiceManager.h"
#include "SmTimeSeriesDBManager.h"
#include "Util/VtStringUtil.h"
//#include "SmUser.h"
//#include "SmUserManager.h"
#include "Json/json.hpp"
#include <vector>
#include "SmHdClient.h"
#include "Log/loguru.hpp"
#include "SmChartData.h"
#include "SmUtil.h"
#include <chrono>
#include "SmChartDataManager.h"
#include "SmSymbolManager.h"
#include "SmSymbol.h"
#include "SmServiceDefine.h"
#include "SmUtfUtil.h"
#include "SmMongoDBManager.h"
#include "SmSessionManager.h"

using namespace std::chrono;
using namespace nlohmann;

SmTimeSeriesServiceManager::SmTimeSeriesServiceManager()
{
	
}

SmTimeSeriesServiceManager::~SmTimeSeriesServiceManager()
{

}

void SmTimeSeriesServiceManager::OnUnregisterCycleDataRequest(SmChartDataRequest&& data_req)
{
	// 기존 목록에 요청이 있는지 확인한다.
	auto it = _CycleDataReqMap.find(data_req.GetDataKey());
	if (it != _CycleDataReqMap.end()) {
		return;
	}

	SmChartData* chart_data = it->second;
	chart_data->RemoveUser(data_req.user_id);
	size_t user_count = chart_data->GetUserCount();
	if (user_count == 0) {
		// 사용자가 없을 경우에는 타이머를 정지시켜 준다.
		auto tit = _CycleDataReqTimerMap.find(data_req.GetDataKey());
		if (tit != _CycleDataReqTimerMap.end()) {
			auto timer_id = tit->second;
			_Timer.remove(timer_id);
			_CycleDataReqTimerMap.erase(tit);
		}
		// 차트 객체도 삭제해 준다.
		_CycleDataReqMap.erase(it);
	}
}

void SmTimeSeriesServiceManager::OnRegisterCycleDataRequest(SmChartDataRequest&& data_req)
{
	auto it = _CycleDataReqMap.find(data_req.GetDataKey());
	// 이미 차트 데이터에 대한 요청이 있는 경우에는 그 차트데이터에 사용자 아이디만 추가한다.
	if (it != _CycleDataReqMap.end()) {
		it->second->AddUser(data_req.user_id);
		return;
	}
	// 차트 데이터를 등록해 준다.
	SmChartData* chart_data = AddCycleDataReq(data_req);
	SmHdClient* client = SmHdClient::GetInstance();
	client->GetChartData(data_req);
	// 차트 데이터 타이머 서비스를 등록해 준다.
	RegisterTimer(chart_data);
}

void SmTimeSeriesServiceManager::OnChartDataRequest(SmChartDataRequest&& data_req)
{
	SmChartDataManager* chartDataMgr = SmChartDataManager::GetInstance();
	SmChartData* chart_data = chartDataMgr->FindChartData(data_req.GetDataKey());
	// 차트 데이터를 찾아봐서 차트 데이터가 없을 경우 혹은 요청 갯수가 클 때만 요청을 한다.
	if (!chart_data) {
		// 차트데이터를 요청한다.
		GetChartDataFromSourceServer(std::move(data_req));
	} 
	else {
		// 주기데이터를 등록해 준다.
		RegisterCycleChartDataRequest(data_req);
		// 차트 데이터 관리자에서 직접 보낸다.
		SendChartData(std::move(data_req), chart_data);
	}
}

void SmTimeSeriesServiceManager::RegisterCycleChartDataRequest(SmChartDataRequest data_req)
{
	auto it = _CycleDataReqMap.find(data_req.GetDataKey());
	// 이미 차트 데이터에 대한 요청이 있는 경우에는 그 차트데이터에 사용자 아이디만 추가한다.
	if (it != _CycleDataReqMap.end()) {
		it->second->AddUser(data_req.user_id);
		return;
	}
	// 차트 데이터를 등록해 준다.
	SmChartData* chart_data = AddCycleDataReq(data_req);
	// 차트 데이터 타이머 서비스를 등록해 준다.
	RegisterTimer(chart_data);
}

void SmTimeSeriesServiceManager::SendChartData(SmChartDataRequest data_req, SmChartData* chart_data)
{
	if (!chart_data)
		return;
	int split_size = _SendDataSplitSize;
	int cur_count = 0;
	int start_index = 0;
	int end_index = 0;
	std::vector<SmSimpleChartDataItem> dataVec;
	std::list<SmChartDataItem>& data = chart_data->GetDataItemList();
	int i = 0;
	for (auto it = data.begin(); it != data.end(); ++it) {
		end_index = i;
		SmChartDataItem srcItem = *it;
		std::string time = srcItem.date + srcItem.time;
		std::string local_date_time = VtStringUtil::GetLocalTimeByDatetimeString(time);
		SmSimpleChartDataItem item;
		item.date_time = local_date_time;
		item.c = srcItem.c;
		item.h = srcItem.h;
		item.l = srcItem.l;
		item.o = srcItem.o;
		item.v = srcItem.v;
		dataVec.push_back(item);
		cur_count++;
		if (cur_count % split_size == 0) {
			SendChartData(dataVec, data_req, data.size(), start_index, end_index);
			if (end_index != data.size() - 1)
				start_index = end_index;
			dataVec.clear();
		}
		i++;
	}
	if (data.size() % split_size != 0) {
		SendChartData(dataVec, data_req, data.size(), start_index, end_index);
	}
}

void SmTimeSeriesServiceManager::OnChartDataReceived(SmChartDataRequest&& data_req)
{
	//OnChartDataRequest(std::move(data_req));
}

void SmTimeSeriesServiceManager::OnSiseDataRequest(SmSiseDataRequest&& sise_req)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = symMgr->FindSymbol(sise_req.symbol_code);
	if (!sym)
		return;
	std::string content = sym->GetQuoteByJson();
	//SmUserManager* userMgr = SmUserManager::GetInstance();
	//userMgr->SendResultMessage(sise_req.user_id, content);
}

void SmTimeSeriesServiceManager::OnHogaDataRequest(SmHogaDataRequest&& hoga_req)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = symMgr->FindSymbol(hoga_req.symbol_code);
	if (!sym)
		return;
	std::string content = sym->GetHogaByJson();
	//SmUserManager* userMgr = SmUserManager::GetInstance();
	//userMgr->SendResultMessage(hoga_req.user_id, content);
}

void SmTimeSeriesServiceManager::OnSymbolMasterRequest(SmSymbolMasterRequest&& master_req)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = symMgr->FindSymbol(master_req.symbol_code);
	if (!sym)
		return;
	json send_object;
	send_object["res_id"] = SmProtocol::res_symbol_master;
	send_object["symbol_code"] = sym->SymbolCode();
	send_object["name_kr"] = SmUtfUtil::AnsiToUtf8((char*)sym->Name().c_str());
	send_object["name_en"] = sym->NameEn().c_str();
	send_object["category_code"] = sym->CategoryCode();
	send_object["market_name"] = SmUtfUtil::AnsiToUtf8((char*)sym->MarketName().c_str());
	send_object["decimal"] = sym->Decimal();
	send_object["contract_unit"] = sym->CtrUnit();
	send_object["seungsu"] = sym->Seungsu();
	send_object["tick_size"] = sym->TickSize();
	send_object["tick_value"] = sym->TickValue();

	std::string content = send_object.dump(4);
	//SmUserManager* userMgr = SmUserManager::GetInstance();
	//userMgr->SendResultMessage(master_req.user_id, content);
}

void SmTimeSeriesServiceManager::OnCompleteChartData(SmChartDataRequest data_req, SmChartData* chart_data)
{
	if (!chart_data)
		return;
	// 차트데이터를 받았음을 표시해 준다.
	//chart_data->Received(true);
	// 주기데이터를 먼저 등록해 준다.
	//RegisterCycleChartDataRequest(data_req);
	// 차트데이터를 보낸다.
	//SendChartData(std::move(data_req), chart_data);
	// 먼저 데이터베이스에 차트데이터를 저장한다.
	SmMongoDBManager* mongoMgr = SmMongoDBManager::GetInstance();
	mongoMgr->SaveChartData(chart_data);
	// 여기서 서버에 차트 데이터가 도착했음을 알린다.
	json send_object;
	send_object["req_id"] = SmProtocol::req_chart_data_resend;
	send_object["service_req_id"] = data_req.service_req_id;
	send_object["session_id"] = data_req.session_id;
	send_object["user_id"] = data_req.user_id;
	send_object["symbol_code"] = data_req.symbolCode;
	send_object["chart_type"] = (int)data_req.chartType;
	send_object["cycle"] = data_req.cycle;
	send_object["count"] = data_req.count;
	std::string content = send_object.dump();
	// 서버에 세션 아이와 함께 클라이언트에게 차트데이터가 수집되었음을 알리라고 보낸다.
	// 서버에 이 메시지가 도착하면 서버는 데이터베이스에서 데이터를 가져와 세션 아이디를 가진 소켓으로 데이터를 보낸다.
	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->Send(content);
	// 주기데이터를 등록해 준다.
	RegisterTimer(chart_data);
}

void SmTimeSeriesServiceManager::OnCompleteChartCycleData(SmChartDataRequest data_req)
{
	json send_object;
	send_object["req_id"] = SmProtocol::req_update_chart_data;
	send_object["symbol_code"] = data_req.symbolCode;
	send_object["chart_type"] = (int)data_req.chartType;
	send_object["cycle"] = data_req.cycle;
	std::string content = send_object.dump();
	// 서버에 세션 아이와 함께 클라이언트에게 차트데이터가 수집되었음을 알리라고 보낸다.
	// 서버에 이 메시지가 도착하면 서버는 데이터베이스에서 데이터를 가져와 세션 아이디를 가진 소켓으로 데이터를 보낸다.
	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->Send(content);
}

void SmTimeSeriesServiceManager::SendChartData(std::vector<SmSimpleChartDataItem>& dataVec, SmChartDataRequest req, int totalCount, int startIndex, int endIndex)
{
	if (dataVec.size() == 0)
		return;

	CString msg;
	json send_object;
	send_object["res_id"] = SmProtocol::res_chart_data;
	send_object["chart_id"] = req.chart_id;
	send_object["symbol_code"] = req.symbolCode;
	send_object["chart_type"] = (int)req.chartType;
	send_object["cycle"] = req.cycle;
	send_object["total_count"] = totalCount;
	send_object["cur_count"] = (int)dataVec.size();
	send_object["start_index"] = startIndex;
	send_object["end_index"] = endIndex;
	for (size_t i = 0; i < dataVec.size(); ++i) {
		SmSimpleChartDataItem item = dataVec[i];
		std::string date = item.date_time;
		send_object["data"][i] = {
			{ "date_time",  date },
			{ "high", item.h },
			{ "low",  item.l },
			{ "open",  item.o },
			{ "close",  item.c },
			{ "volume",  item.v }
		};
		
		msg.Format("SendChartData :: date_time = %s\n", date.c_str());
		//TRACE(msg);
	}

	std::string content = send_object.dump();
	//SmUserManager* userMgr = SmUserManager::GetInstance();
	//userMgr->SendResultMessage(req.user_id, content);
}

SmChartData* SmTimeSeriesServiceManager::AddCycleDataReq(SmChartDataRequest data_req)
{
	SmChartDataManager* chrartDataMgr = SmChartDataManager::GetInstance();
	SmChartData* chartData = chrartDataMgr->AddChartData(data_req);
	chartData->AddUser(data_req.user_id);
	_CycleDataReqMap[data_req.GetDataKey()] = chartData;

	return chartData;
}

void SmTimeSeriesServiceManager::RegisterTimer(SmChartData* chartData)
{
	if (!chartData)
		return;
	// 주기 데이터는 분 데이터만 처리한다. 
	if (chartData->ChartType() != SmChartType::MIN)
		return;
	// 대기시간 
	int waitTime = 0;
	int cycle_time = chartData->Cycle() * 60;
	// 주기가 60분을 넘으면 60분마다 갱신한다.
	if (chartData->Cycle() > 60) {
		cycle_time = 60 * 60;
	}
	else {
		std::vector<int> date_time = SmUtil::GetLocalDateTime();
		int cur_min = date_time[4];
		int total_seconds = cur_min * 60 + date_time[5];
		int mod_seconds = total_seconds % (chartData->Cycle() * 60);
		int wait_seconds = chartData->Cycle() * 60 - mod_seconds;
		waitTime = wait_seconds;
	}
	
	// 주기를 초로 환산해서 대입한다.
	// Add to the timer.
	auto id = _Timer.add(seconds(waitTime), std::bind(&SmChartData::OnTimer, chartData), seconds(cycle_time));
	// Add to the request map.
	_CycleDataReqTimerMap[chartData->GetDataKey()] = id;
}

void SmTimeSeriesServiceManager::SendChartDataFromDB(SmChartDataRequest&& data_req)
{
	SmTimeSeriesDBManager* dbMgr = SmTimeSeriesDBManager::GetInstance();
	std::string query_string;// = "show series on abroad_future";
	query_string.append("SELECT \"c\", \"h\", \"l\", \"o\", \"v\" , \"local_date\" , \"local_time\" FROM \"");
	query_string.append(data_req.GetDataKey());
	query_string.append("\" ");
	query_string.append("LIMIT ");
	query_string.append(std::to_string(data_req.count));
	//query_string.append("where time >= '2019-06-05T07:12:00Z'");
	std::string resp = dbMgr->ExecQuery(query_string);
	CString msg;
	msg.Format("resp len = %d", resp.length());
	//TRACE(msg);
	SmChartDataManager* chartDataMgr = SmChartDataManager::GetInstance();
	SmChartData* chart_data = chartDataMgr->AddChartData(data_req);
	try
	{
		auto json_object = json::parse(resp);
		auto it = json_object.find("error");
		if (it != json_object.end()) {
			std::string err_msg = json_object["error"];
			//TRACE(err_msg.c_str());
			LOG_F(INFO, "Query Error", err_msg);
			return;
		}
		auto series = json_object["results"][0]["series"];
		if (series.is_null()) {
			_HistoryDataReqMap[data_req.GetDataKey()] = data_req;
			SmHdClient* client = SmHdClient::GetInstance();
			client->GetChartData(data_req);
			return;
		}
		auto a = json_object["results"][0]["series"][0]["values"];
		int split_size = _SendDataSplitSize;
		int cur_count = 0;
		int start_index = 0;
		int end_index = 0;
		std::vector<SmSimpleChartDataItem> dataVec;
		//SmUserManager* userMgr = SmUserManager::GetInstance();
		for (size_t i = 0; i < a.size(); i++) {
			end_index = i;
			auto val = a[i];
			std::string time = val[0];
			std::string local_date_time = VtStringUtil::GetLocalTime(time);
			SmSimpleChartDataItem item;
			item.date_time = local_date_time;
			item.c = val[1];
			item.h = val[2];
			item.l = val[3];
			item.o = val[4];
			item.v = val[5];
			std::string local_date = "";
			std::string local_time = "";
			if (!val[6].is_null())
				local_date = val[6];
			if (!val[7].is_null())
				local_time = val[7];
			msg.Format(_T("index = %d, datetime = %s, o = %d, h = %d, l = %d, c = %d, v = %d\n"), i, local_date_time.c_str(), item.o, item.h, item.l, item.c, item.v);
			TRACE(msg);

			SmChartDataItem data;
			data.symbolCode = data_req.symbolCode;
			data.chartType = data_req.chartType;
			data.cycle = data_req.cycle;
			data.date = local_date;
			data.time = local_time;
			data.h = item.h;
			data.l = item.l;
			data.o = item.o;
			data.c = item.c;
			data.v = item.v;
			chart_data->PushChartDataItemToBack(data);

			dataVec.push_back(item);
			cur_count++;
			if (cur_count % split_size == 0) {
				SendChartData(dataVec, data_req, a.size(), start_index, end_index);
				if (end_index != a.size() - 1)
					start_index = end_index;
				dataVec.clear();
			}
		}
		if (a.size() % split_size != 0) {
			SendChartData(dataVec, data_req, a.size(), start_index, end_index);
		}
	}
	catch (const std::exception& e)
	{
		std::string error = e.what();
		LOG_F(INFO, "Query Error", error);
	}
}

void SmTimeSeriesServiceManager::GetChartDataFromSourceServer(SmChartDataRequest&& data_req)
{
	SmHdClient* client = SmHdClient::GetInstance();
	client->GetChartData(data_req);
}
