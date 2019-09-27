#pragma once
#include "Global/TemplateSingleton.h"
#include "SmChartDefine.h"
#include <vector>
#include <map>
#include <string>
#include "Timer/cpptime.h"

class SmChartData;
class SmTimeSeriesServiceManager : public TemplateSingleton<SmTimeSeriesServiceManager>
{
public:
	SmTimeSeriesServiceManager();
	~SmTimeSeriesServiceManager();
	void OnUnregisterCycleDataRequest(SmChartDataRequest&& data_req);
	void OnRegisterCycleDataRequest(SmChartDataRequest&& data_req);
	void OnChartDataRequest(SmChartDataRequest&& data_req);
	void OnChartDataReceived(SmChartDataRequest&& data_req);
	void OnSiseDataRequest(SmSiseDataRequest&& sise_req);
	void OnHogaDataRequest(SmHogaDataRequest&& hoga_req);
	void OnSymbolMasterRequest(SmSymbolMasterRequest&& master_req);
	int SendDataSplitSize() const { return _SendDataSplitSize; }
	void SendDataSplitSize(int val) { _SendDataSplitSize = val; }
	void OnCompleteChartData(SmChartDataRequest data_req, SmChartData* chart_data);
	void OnCompleteChartCycleData(SmChartDataRequest data_req);
	void SendChartData(int req_session_id, int total_count, int current_count, SmChartDataItem& item);
	void SendCycleChartData(SmChartDataItem& item);
private:
	void RegisterCycleChartDataRequest(SmChartDataRequest data_req);
	void SendChartData(std::vector<SmSimpleChartDataItem>& dataVec, SmChartDataRequest req, int totalCount, int startIndex, int endIndex);
	std::map<std::string, SmChartDataRequest> _HistoryDataReqMap;
	std::map<std::string, CppTime::timer_id> _CycleDataReqTimerMap;
	int _SendDataSplitSize = 20;
	CppTime::Timer _Timer;
	std::map<std::string, SmChartData*> _CycleDataReqMap;
	SmChartData* AddCycleDataReq(SmChartDataRequest data_req);
	void RegisterTimer(SmChartData* chartData);
	void SendChartDataFromDB(SmChartDataRequest&& data_req);
	// 증권사나 데이터 제공 업체에 직접 데이터를 요청한다.
	void GetChartDataFromSourceServer(SmChartDataRequest&& data_req);
	void SendChartData(SmChartDataRequest data_req, SmChartData* chart_data);
};

