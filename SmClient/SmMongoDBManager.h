#pragma once
#include "Global/TemplateSingleton.h"
#include <mongocxx\config\config.hpp>
namespace mongocxx
{
	MONGOCXX_INLINE_NAMESPACE_BEGIN
		class instance;
		class client;
	MONGOCXX_INLINE_NAMESPACE_END
};

class SmMongoDBManager : public TemplateSingleton<SmMongoDBManager>
{
public:
	SmMongoDBManager();
	~SmMongoDBManager();
	void Test();
	void ReadSymbol();
private:
	void SaveSymbolsToDatabase();
	void InitDatabase();
	mongocxx::instance* _Instance = nullptr;
	mongocxx::client* _Client = nullptr;
};
