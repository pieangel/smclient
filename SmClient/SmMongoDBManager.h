#pragma once
#include "Global/TemplateSingleton.h"
#include <mongocxx\config\config.hpp>
#include "SmHogaDefine.h"
#include "SmQuoteDefine.h"
namespace mongocxx
{
	MONGOCXX_INLINE_NAMESPACE_BEGIN
		class instance;
		class client;
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
private:
	void SaveMarketsToDatabase();
	void SaveSymbolsToDatabase();
	void InitDatabase();
	mongocxx::instance* _Instance = nullptr;
	mongocxx::client* _Client = nullptr;
};
