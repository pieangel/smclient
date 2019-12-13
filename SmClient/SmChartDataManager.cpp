#include "pch.h"
#include "SmChartDataManager.h"
#include "SmChartData.h"
#include "SmUtil.h"
#include <chrono>
#include <sstream>
#include <fstream>
#include "SmConfigManager.h"
#include "Log/loguru.hpp"
#include "Util/VtStringUtil.h"
#include "SmMongoDBManager.h"

using namespace std::chrono;

SmChartDataManager::SmChartDataManager()
{
	InitSymbolConvertMap();
}

SmChartDataManager::~SmChartDataManager()
{
	for (auto it = _ChartDataMap.begin(); it != _ChartDataMap.end(); ++it) {
		delete it->second;
	}
}

void SmChartDataManager::AddChartData(SmChartData* chart_data)
{
	if (!chart_data)
		return;

	auto it = _ChartDataMap.find(chart_data->GetDataKey());
	if (it == _ChartDataMap.end()) {
		_ChartDataMap[chart_data->GetDataKey()] = chart_data;
	}
}

SmChartData* SmChartDataManager::AddChartData(SmChartDataRequest data_req)
{
	SmChartData* chartData = FindChartData(data_req.GetDataKey());
	if (!chartData) {
		chartData = new SmChartData();
		chartData->SymbolCode(data_req.symbolCode);
		chartData->ChartType(data_req.chartType);
		chartData->Cycle(data_req.cycle);
		_ChartDataMap[data_req.GetDataKey()] = chartData;
	}

	return chartData;
}

SmChartData* SmChartDataManager::AddChartData(SmChartDataItem data_item)
{
	SmChartData* chartData = FindChartData(data_item.GetDataKey());
	if (!chartData) {
		chartData = new SmChartData();
		chartData->SymbolCode(data_item.symbolCode);
		chartData->ChartType(data_item.chartType);
		chartData->Cycle(data_item.cycle);
		_ChartDataMap[data_item.GetDataKey()] = chartData;
	}

	return chartData;
}

SmChartData* SmChartDataManager::AddChartData(std::string symbol_code, int chart_type, int cycle)
{
	std::string data_key = SmChartData::MakeDataKey(symbol_code, chart_type, cycle);
	SmChartData* chartData = FindChartData(data_key);
	if (!chartData) {
		chartData = new SmChartData();
		chartData->SymbolCode(symbol_code);
		chartData->ChartType((SmChartType)chart_type);
		chartData->Cycle(cycle);
		_ChartDataMap[data_key] = chartData;
	}

	return chartData;
}

void SmChartDataManager::RegisterTimer(SmChartData* chartData)
{
	if (!chartData)
		return;

	auto it = _CycleDataReqTimerMap.find(chartData->GetDataKey());
	// 이미 등록되어 있으면 등록하지 않는다.
	if (it != _CycleDataReqTimerMap.end())
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

void SmChartDataManager::ReadDailyChartData()
{
	SmConfigManager* configMgr = SmConfigManager::GetInstance();

	std::string appPath;
	appPath = configMgr->GetApplicationPath();
	appPath.append("\\");
	appPath.append("chart_data");
	appPath.append("\\");
	std::string config_path = appPath;
	std::map<std::string, std::string> file_list;
	ListContents(file_list, config_path, "*.txt", false);
	for (auto it = file_list.begin(); it != file_list.end(); ++it) {
		std::string product_code = it->first;
		std::string file_name = it->second;
		std::ifstream infile(file_name);
		std::string line;
		while (std::getline(infile, line)) {
			std::istringstream iss(line);
			std::vector<std::string> split_result;
			split_result = VtStringUtil::Split(line, ',');
			if (split_result.size() > 0) {
				int decimal = FindDecimal(split_result);
				SmChartDataItem item;
				item.chartType = SmChartType::DAY;
				item.symbolCode = product_code;
				item.cycle = 1;
				item.date = split_result[1];
				item.time = "090000";
				std::string value = split_result[2];
				item.o = std::stod(value) * std::pow(10, decimal);
				value = split_result[3];
				item.h = std::stod(value) * std::pow(10, decimal);
				value = split_result[4];
				item.l = std::stod(value) * std::pow(10, decimal);
				value = split_result[5];
				item.c = std::stod(value) * std::pow(10, decimal);
				item.v = std::stoi(split_result[6]);
				SmMongoDBManager::GetInstance()->SaveChartDataItem(item);
			}
		}
	}
}

bool SmChartDataManager::ListContents(std::map<std::string, std::string>& dest, std::string dir, std::string filter, bool recursively)
{
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	// Prepare string
	//if (dir.back() != '\\') dir += "\\";

	// Safety check
	if (dir.length() >= MAX_PATH) {
		LOG_F(INFO, "Cannot open folder %s: path too long", dir.c_str());
		return false;
	}

	// First entry in directory
	hFind = FindFirstFileA((dir + filter).c_str(), &ffd);

	if (hFind == INVALID_HANDLE_VALUE) {
		LOG_F(INFO, "Cannot open folder in folder %s: error accessing first entry.", dir.c_str());
		return false;
	}

	// List files in directory
	do {
		// Ignore . and .. folders, they cause stack overflow
		if (strcmp(ffd.cFileName, ".") == 0) continue;
		if (strcmp(ffd.cFileName, "..") == 0) continue;


		// Is directory?
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// Go inside recursively
			//if (recursively)
			//	ListContents(dest, dir + ffd.cFileName, filter, recursively, content_type);
			continue;
		}
		// Add file to our list
		else {

			SYSTEMTIME stLocal;

			// Convert the last-write time to local time.
			FileTimeToSystemTime(&ffd.ftLastWriteTime, &stLocal);
			std::string local_time;
			local_time += std::to_string(stLocal.wYear);
			local_time += std::to_string(stLocal.wMonth);
			local_time += std::to_string(stLocal.wDay);
			std::string file_name = ffd.cFileName;
			int pos = file_name.find("_");
			if (pos > 0) {
				std::string name = file_name.substr(0, pos);
				auto it = _SymbolConvertMap.find(name);
				if (it != _SymbolConvertMap.end()) {
					name = it->second;
				}
				dest[name] = dir + ffd.cFileName;
			}
			else {
				std::string name = file_name.substr(0, 3);
				if (name.compare("000") == 0) {
					name = "101";
				}
				dest[name] = dir + ffd.cFileName;
			}
		}

	} while (FindNextFileA(hFind, &ffd));

	// Get last error
	dwError = GetLastError();
	if (dwError != ERROR_NO_MORE_FILES) {
		LOG_F(INFO, "Error reading file list in folder %s.", dir.c_str());
		return false;
	}

	return true;
}

void SmChartDataManager::InitSymbolConvertMap()
{
	_SymbolConvertMap["FSMI"] = "SMI";
	_SymbolConvertMap["FDX"] = "GX";
	_SymbolConvertMap["FESX"] = "VG";
	_SymbolConvertMap["HCEI"] = "HHI";
	_SymbolConvertMap["SFC"] = "CN";
	_SymbolConvertMap["SIN"] = "IN";
	_SymbolConvertMap["SNK"] = "NK";
	_SymbolConvertMap["STW"] = "TW";
}

int SmChartDataManager::FindDecimal(std::vector<std::string> vec)
{
	int decimal = 5;
	int min_pos = 5;
	for (int i = 2; i <= 5; i++) {
		std::string value = vec[i];
		int pos = 0;
		for (int j = value.size() - 1; j >= 0; --j) {
			char cur_char = value[j];
			// 소수점이나 0이 아닌 문자가 나오면 루프에서 벗어난다.
			if (cur_char == '.' || cur_char != '0')
				break;
			pos++;
		}
		if (pos < min_pos)
			min_pos = pos;
	}

	return decimal - min_pos;
}

SmChartData* SmChartDataManager::FindChartData(std::string data_key)
{
	auto it = _ChartDataMap.find(data_key);
	if (it != _ChartDataMap.end()) {
		return it->second;
	}

	return nullptr;
}

