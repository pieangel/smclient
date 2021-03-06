#pragma once
#include "Global/TemplateSingleton.h"
#include <mongocxx\config\config.hpp>
#include "SmHogaDefine.h"
#include "SmQuoteDefine.h"
#include "SmChartDefine.h"
namespace mongocxx
{
	MONGOCXX_INLINE_NAMESPACE_BEGIN
		class instance;
		class client;
		class pool;
	MONGOCXX_INLINE_NAMESPACE_END
};

class SmChartData;
class SmMongoDBManager : public TemplateSingleton<SmMongoDBManager>
{
public:
	SmMongoDBManager();
	~SmMongoDBManager();
	void Test();
	void ReadSymbol();
	void SaveChartData(SmChartData* chart_data);
	void SaveHoga(SmHoga hoga);
	void SaveSise(SmQuote quote);
	void SaveChartDataItem(SmChartDataItem item);
	void LoadChartDataRequest();
private:
	void SaveMarketsToDatabase();
	void SaveSymbolsToDatabase();
	void InitDatabase();
	mongocxx::instance* _Instance = nullptr;
	mongocxx::client* _Client = nullptr;
	mongocxx::pool* _ConnPool = nullptr;
};
