#include "SmMongoDBManager.h"
#include <chrono>

#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <string>
#include <iostream>
#include "SmMarketManager.h"
#include "SmSymbolReader.h"
#include "SmConfigManager.h"
#include "SmUtfUtil.h"
#include "SmMarket.h"
#include "SmCategory.h"
#include "SmSymbol.h"
#include <codecvt>
#include <locale>
#include "SmChartData.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

SmMongoDBManager::SmMongoDBManager()
{
	InitDatabase();
}

SmMongoDBManager::~SmMongoDBManager()
{
	if (_Client) {
		delete _Client;
		_Client = nullptr;
	}

	if (_Instance) {
		delete _Instance;
		_Instance = nullptr;
	}
}

void SmMongoDBManager::Test()
{
	if (!_Client)
		return;

	auto db = (*_Client)["andromeda"];

	// TODO: fix dates

	std::string name = "company";

	// @begin: cpp-insert-a-document
	bsoncxx::document::value restaurant_doc = make_document(
		kvp("address",
			make_document(kvp("street", "2 Avenue"),
				kvp("zipcode", 10075),
				kvp("building", "1480"),
				kvp("coord", make_array(-73.9557413, 40.7720266)))),
		kvp("borough", "Manhattan"),
		kvp("cuisine", "Italian"),
		kvp("grades",
			make_array(
				make_document(kvp("date", bsoncxx::types::b_date{ std::chrono::milliseconds{12323} }),
					kvp("grade", "A"),
					kvp("score", 11)),
				make_document(
					kvp("date", bsoncxx::types::b_date{ std::chrono::milliseconds{121212} }),
					kvp("grade", "B"),
					kvp("score", 17)))),
		kvp("name", name),
		kvp("restaurant_id", "41704623"));

	// We choose to move in our document here, which transfers ownership to insert_one()
	auto res = db["andromeda"].insert_one(std::move(restaurant_doc));
	// @end: cpp-insert-a-document

	auto builder = bsoncxx::builder::stream::document{};
	bsoncxx::document::value doc_value = builder
		<< "name" << "MongoDB"
		<< "type" << "database"
		<< "count" << 1
		<< "versions" << bsoncxx::builder::stream::open_array
		<< "v3.2" << "v3.0" << "v2.6"
		<< close_array
		<< "info" << bsoncxx::builder::stream::open_document
		<< "x" << 203
		<< "y" << 102
		<< bsoncxx::builder::stream::close_document
		<< bsoncxx::builder::stream::finalize;

	res = db["database"].insert_one(std::move(doc_value));


	std::vector<bsoncxx::document::value> documents;
	for (int i = 0; i < 100; i++) {
		documents.push_back(
			bsoncxx::builder::stream::document{} << "i" << i << finalize);
	}

	mongocxx::collection coll = db["test"];
	coll.insert_many(documents);


	bsoncxx::stdx::optional<bsoncxx::document::value> maybe_result =
		coll.find_one(document{} << "i" << 18 << finalize);
	if (maybe_result) {
		std::cout << bsoncxx::to_json(*maybe_result) << "\n";
	}



	mongocxx::cursor cursor = coll.find(
		document{} << "i" << open_document <<
		"$gt" << 5 <<
		"$lte" << 10
		<< close_document << finalize);
	for (auto doc : cursor) {
		std::cout << bsoncxx::to_json(doc) << "\n";
	}



	coll.update_one(document{} << "i" << 10 << finalize,
		document{} << "$set" << open_document <<
		"i" << 110 << close_document << finalize);

	bsoncxx::stdx::optional<mongocxx::result::update> result =
		coll.update_many(
			document{} << "i" << open_document <<
			"$lt" << 100 << close_document << finalize,
			document{} << "$inc" << open_document <<
			"i" << 100 << close_document << finalize);

	if (result) {
		std::cout << result->modified_count() << "\n";
	}

}

void SmMongoDBManager::ReadSymbol()
{
	SmSymbolReader* symReader = SmSymbolReader::GetInstance();
	std::string dir = symReader->GetWorkingDir();
	std::string name = dir;
	SmMarketManager* mrktMgr = SmMarketManager::GetInstance();

	//DbTest();

	SmConfigManager* configMgr = SmConfigManager::GetInstance();
	std::string appPath = configMgr->GetApplicationPath();

	mrktMgr->ReadSymbolsFromFile();

	SaveMarketsToDatabase();

	SaveSymbolsToDatabase();
}

void SmMongoDBManager::SaveChartData(SmChartData* chart_data)
{
	try
	{
		if (!chart_data)
			return;

		auto db = (*_Client)["andromeda"];
		using namespace bsoncxx;

		std::string data_key = chart_data->GetDataKey();

		// 먼저 시장이 있는지 검색한다. 
		// 그리고 시장 속에 상품이 있는지 검색한다.
		mongocxx::collection chart_coll = db["chart_data"];

		builder::stream::document builder{};

		bsoncxx::stdx::optional<bsoncxx::document::value> found_chart_data =
			chart_coll.find_one(bsoncxx::builder::stream::document{} << "data_key" << chart_data->GetDataKey() << finalize);
		// 차트 데이터가 없을 경우
		if (!found_chart_data) {
			// 차트 데이터에 대한 정보를 저장한다.
			bsoncxx::document::value doc_chart_data = builder
				<< "data_key" << data_key
				<< "symbol_code" << chart_data->SymbolCode()
				<< "chart_type" << (int)chart_data->ChartType()
				<< "chart_cycle" << chart_data->Cycle()
				<< bsoncxx::builder::stream::finalize;

			auto res = db["chart_data"].insert_one(std::move(doc_chart_data));


			// 차트 데이터만 따로 저장한다.
			mongocxx::collection data_coll = db[data_key];
			std::list<SmChartDataItem>& data_list = chart_data->GetDataItemList();
			for (auto it = data_list.begin(); it != data_list.end(); ++it) {
				SmChartDataItem item = *it;
				std::string date_time = item.date + item.time;
				bsoncxx::document::value doc_chart_data_item = builder
					<< "date_time" << date_time
					<< "local_date" << item.date
					<< "local_time" << item.time
					<< "o" << item.o
					<< "h" << item.h
					<< "l" << item.l
					<< "c" << item.c
					<< "v" << item.v
					<< bsoncxx::builder::stream::finalize;
				auto res = db[data_key].insert_one(std::move(doc_chart_data_item));
			}
		} 
		else {
			// 차트 데이터만 따로 저장한다.
			mongocxx::collection data_coll = db[data_key];
			std::list<SmChartDataItem>& data_list = chart_data->GetDataItemList();
			for (auto it = data_list.begin(); it != data_list.end(); ++it) {
				SmChartDataItem item = *it;
				std::string date_time = item.date + item.time;
				bsoncxx::stdx::optional<bsoncxx::document::value> found_item = data_coll.find_one(bsoncxx::builder::stream::document{} << "date_time" << date_time << finalize);
				// 같은 날짜가 없으면 새로 추가 한다.
				if (!found_item) {
					bsoncxx::document::value doc_chart_data_item = builder
						<< "date_time" << date_time
						<< "local_date" << item.date
						<< "local_time" << item.time
						<< "o" << item.o
						<< "h" << item.h
						<< "l" << item.l
						<< "c" << item.c
						<< "v" << item.v
						<< bsoncxx::builder::stream::finalize;
					auto res = db[data_key].insert_one(std::move(doc_chart_data_item));
				}
				else { // 같은 날짜가 있으면 업데이트 한다.
					data_coll.update_one(bsoncxx::builder::stream::document{} << "date_time" << date_time << finalize,
						bsoncxx::builder::stream::document{} << "$set" 
						<< open_document 
						<< "local_date" << item.date
						<< "local_time" << item.time
						<< "o" << item.o
						<< "h" << item.h
						<< "l" << item.l
						<< "c" << item.c
						<< "v" << item.v
						<< close_document << finalize);
				}
			}
		}
	}
	catch (std::exception e) {
		std::string error;
		error = e.what();
	}
}

void SmMongoDBManager::SaveHoga(SmHoga hoga)
{
	try
	{
		auto db = (*_Client)["andromeda"];
		using namespace bsoncxx;

		std::string symbol_code = hoga.SymbolCode;

		// 먼저 시장이 있는지 검색한다. 
		// 그리고 시장 속에 상품이 있는지 검색한다.
		mongocxx::collection hoga_coll = db["hoga"];

		builder::stream::document builder{};

		/*
		bsoncxx::stdx::optional<bsoncxx::document::value> found_hoga =
			hoga_coll.find_one(bsoncxx::builder::stream::document{} << "symbol_code" << symbol_code << finalize);
		// 호가 데이터가 없을 경우
		if (!found_hoga) {
			auto in_array = builder << "hoga_item" << builder::stream::open_array;
			for (size_t j = 0; j < 5; ++j) {
				in_array = in_array << builder::stream::open_document
					<< "item_index" << (int)j
					<< "buy_price" << hoga.Ary[j].BuyPrice
					<< "buy_qty" << hoga.Ary[j].BuyQty
					<< "buy_cnt" << hoga.Ary[j].BuyCnt
					<< "sell_price" << hoga.Ary[j].SellPrice
					<< "sell_qty" << hoga.Ary[j].SellQty
					<< "sell_cnt" << hoga.Ary[j].SellCnt
					<< builder::stream::close_document;
			}
			auto after_array = in_array << builder::stream::close_array;
			after_array 
				<< "tot_buy_qty" << hoga.TotBuyQty
				<< "tot_buy_cnt" << hoga.TotBuyCnt
				<< "tot_sell_qty" << hoga.TotSellQty
				<< "tot_sell_cnt" << hoga.TotSellCnt
				<< "date" << hoga.DomesticDate
				<< "time" << hoga.DomesticTime
				<< bsoncxx::builder::stream::finalize;

			bsoncxx::document::value doc = after_array << builder::stream::finalize;
			auto res = db["hoga"].insert_one(std::move(doc));
		}

		else {
			auto in_array = builder << "hoga_item" << builder::stream::open_array;
			for (size_t j = 0; j < 5; ++j) {
				in_array = in_array << builder::stream::open_document
					<< "item_index" << (int)j
					<< "buy_price" << hoga.Ary[j].BuyPrice
					<< "buy_qty" << hoga.Ary[j].BuyQty
					<< "buy_cnt" << hoga.Ary[j].BuyCnt
					<< "sell_price" << hoga.Ary[j].SellPrice
					<< "sell_qty" << hoga.Ary[j].SellQty
					<< "sell_cnt" << hoga.Ary[j].SellCnt
					<< builder::stream::close_document;
			}
			auto after_array = in_array << builder::stream::close_array;
			after_array
				<< "tot_buy_qty" << hoga.TotBuyQty
				<< "tot_buy_cnt" << hoga.TotBuyCnt
				<< "tot_sell_qty" << hoga.TotSellQty
				<< "tot_sell_cnt" << hoga.TotSellCnt
				<< "date" << hoga.DomesticDate
				<< "time" << hoga.DomesticTime
				<< bsoncxx::builder::stream::finalize;

			hoga_coll.update_one(bsoncxx::builder::stream::document{} << "symbol_code" << symbol_code << finalize,
				bsoncxx::builder::stream::document{} << "$set"
				<< open_document
				<< after_array << close_document << finalize);
		}
		*/
		
		bsoncxx::stdx::optional<bsoncxx::document::value> found_hoga =
			hoga_coll.find_one(bsoncxx::builder::stream::document{} << "symbol_code" << symbol_code << finalize);
		// 차트 데이터가 없을 경우
		if (!found_hoga) {
			// 차트 데이터에 대한 정보를 저장한다.
			bsoncxx::document::value doc_chart_data = builder
				<< "symbol_code" << symbol_code

				<< "buy_price1" << hoga.Ary[0].BuyPrice
				<< "buy_qty1" << hoga.Ary[0].BuyQty
				<< "buy_cnt1" << hoga.Ary[0].BuyCnt
				<< "sell_price1" << hoga.Ary[0].SellPrice
				<< "sell_qty1" << hoga.Ary[0].SellQty
				<< "sell_cnt1" << hoga.Ary[0].SellCnt

				<< "buy_price2" << hoga.Ary[1].BuyPrice
				<< "buy_qty2" << hoga.Ary[1].BuyQty
				<< "buy_cnt2" << hoga.Ary[1].BuyCnt
				<< "sell_price2" << hoga.Ary[1].SellPrice
				<< "sell_qty2" << hoga.Ary[1].SellQty
				<< "sell_cnt2" << hoga.Ary[1].SellCnt

				<< "buy_price3" << hoga.Ary[2].BuyPrice
				<< "buy_qty3" << hoga.Ary[2].BuyQty
				<< "buy_cnt3" << hoga.Ary[2].BuyCnt
				<< "sell_price3" << hoga.Ary[2].SellPrice
				<< "sell_qty3" << hoga.Ary[2].SellQty
				<< "sell_cnt3" << hoga.Ary[2].SellCnt

				<< "buy_price4" << hoga.Ary[3].BuyPrice
				<< "buy_qty4" << hoga.Ary[3].BuyQty
				<< "buy_cnt4" << hoga.Ary[3].BuyCnt
				<< "sell_price4" << hoga.Ary[3].SellPrice
				<< "sell_qty4" << hoga.Ary[3].SellQty
				<< "sell_cnt4" << hoga.Ary[3].SellCnt

				<< "buy_price5" << hoga.Ary[4].BuyPrice
				<< "buy_qty5" << hoga.Ary[4].BuyQty
				<< "buy_cnt5" << hoga.Ary[4].BuyCnt
				<< "sell_price5" << hoga.Ary[4].SellPrice
				<< "sell_qty5" << hoga.Ary[4].SellQty
				<< "sell_cnt5" << hoga.Ary[4].SellCnt

				<< "tot_buy_qty" << hoga.TotBuyQty
				<< "tot_buy_cnt" << hoga.TotBuyCnt
				<< "tot_sell_qty" << hoga.TotSellQty
				<< "tot_sell_cnt" << hoga.TotSellCnt
				<< "domestic_date" << hoga.DomesticDate
				<< "domestic_time" << hoga.DomesticTime

				<< bsoncxx::builder::stream::finalize;

			auto res = db["hoga"].insert_one(std::move(doc_chart_data));
		}
		else {
			hoga_coll.update_one(bsoncxx::builder::stream::document{} << "symbol_code" << symbol_code << finalize,
				bsoncxx::builder::stream::document{} << "$set"
				<< open_document
				<< "symbol_code" << symbol_code

				<< "buy_price1" << hoga.Ary[0].BuyPrice
				<< "buy_qty1" << hoga.Ary[0].BuyQty
				<< "buy_cnt1" << hoga.Ary[0].BuyCnt
				<< "sell_price1" << hoga.Ary[0].SellPrice
				<< "sell_qty1" << hoga.Ary[0].SellQty
				<< "sell_cnt1" << hoga.Ary[0].SellCnt

				<< "buy_price2" << hoga.Ary[1].BuyPrice
				<< "buy_qty2" << hoga.Ary[1].BuyQty
				<< "buy_cnt2" << hoga.Ary[1].BuyCnt
				<< "sell_price2" << hoga.Ary[1].SellPrice
				<< "sell_qty2" << hoga.Ary[1].SellQty
				<< "sell_cnt2" << hoga.Ary[1].SellCnt

				<< "buy_price3" << hoga.Ary[2].BuyPrice
				<< "buy_qty3" << hoga.Ary[2].BuyQty
				<< "buy_cnt3" << hoga.Ary[2].BuyCnt
				<< "sell_price3" << hoga.Ary[2].SellPrice
				<< "sell_qty3" << hoga.Ary[2].SellQty
				<< "sell_cnt3" << hoga.Ary[2].SellCnt

				<< "buy_price4" << hoga.Ary[3].BuyPrice
				<< "buy_qty4" << hoga.Ary[3].BuyQty
				<< "buy_cnt4" << hoga.Ary[3].BuyCnt
				<< "sell_price4" << hoga.Ary[3].SellPrice
				<< "sell_qty4" << hoga.Ary[3].SellQty
				<< "sell_cnt4" << hoga.Ary[3].SellCnt

				<< "buy_price5" << hoga.Ary[4].BuyPrice
				<< "buy_qty5" << hoga.Ary[4].BuyQty
				<< "buy_cnt5" << hoga.Ary[4].BuyCnt
				<< "sell_price5" << hoga.Ary[4].SellPrice
				<< "sell_qty5" << hoga.Ary[4].SellQty
				<< "sell_cnt5" << hoga.Ary[4].SellCnt

				<< "tot_buy_qty" << hoga.TotBuyQty
				<< "tot_buy_cnt" << hoga.TotBuyCnt
				<< "tot_sell_qty" << hoga.TotSellQty
				<< "tot_sell_cnt" << hoga.TotSellCnt
				<< "domestic_date" << hoga.DomesticDate
				<< "domestic_time" << hoga.DomesticTime
				<< close_document << finalize);
		}
		
	}
	catch (std::exception e) {
		std::string error;
		error = e.what();
	}
}


void SmMongoDBManager::SaveSise(SmQuote quote)
{
	try
	{
		auto db = (*_Client)["andromeda"];
		using namespace bsoncxx;

		std::string symbol_code = quote.SymbolCode;

		// 먼저 시장이 있는지 검색한다. 
		// 그리고 시장 속에 상품이 있는지 검색한다.
		mongocxx::collection quote_coll = db["quote"];

		builder::stream::document builder{};

		bsoncxx::stdx::optional<bsoncxx::document::value> found_quote =
			quote_coll.find_one(bsoncxx::builder::stream::document{} << "symbol_code" << symbol_code << finalize);
		
		if (!found_quote) {
			// 차트 데이터에 대한 정보를 저장한다.
			bsoncxx::document::value doc_chart_data = builder
				<< "symbol_code" << symbol_code
				<< "sign_to_preday" << quote.SignToPreDay
				<< "gap_from_preday" << quote.GapFromPreDay
				<< "ratio_to_preday" << quote.RatioToPreday
				<< "acc_volume" << quote.accVolume
								
				<< "open" << quote.Open
				<< "high" << quote.High
				<< "low" << quote.Low
				<< "close" << quote.Close
				
				<< bsoncxx::builder::stream::finalize;

			auto res = db["quote"].insert_one(std::move(doc_chart_data));
		}
		else {
			quote_coll.update_one(bsoncxx::builder::stream::document{} << "symbol_code" << symbol_code << finalize,
				bsoncxx::builder::stream::document{} << "$set"
				<< open_document

				<< "symbol_code" << symbol_code
				<< "sign_to_preday" << quote.SignToPreDay
				<< "gap_from_preday" << quote.GapFromPreDay
				<< "ratio_to_preday" << quote.RatioToPreday
				<< "acc_volume" << quote.accVolume
				
				<< "open" << quote.Open
				<< "high" << quote.High
				<< "low" << quote.Low
				<< "close" << quote.Close

				<< close_document << finalize);
		}
	}
	catch (std::exception e) {
		std::string error;
		error = e.what();
	}
}

void SmMongoDBManager::SaveChartDataItem(SmChartDataItem item)
{
	try
	{

		auto db = (*_Client)["andromeda"];
		using namespace bsoncxx;

		std::string data_key = item.GetDataKey();

		// 먼저 시장이 있는지 검색한다. 
		// 그리고 시장 속에 상품이 있는지 검색한다.
		mongocxx::collection chart_coll = db["chart_data"];

		builder::stream::document builder{};

		bsoncxx::stdx::optional<bsoncxx::document::value> found_chart_data =
			chart_coll.find_one(bsoncxx::builder::stream::document{} << "data_key" << item.GetDataKey() << finalize);
		// 차트 데이터가 없을 경우
		if (found_chart_data) {
			// 차트 데이터만 따로 저장한다.
			mongocxx::collection data_coll = db[data_key];
			std::string date_time = item.date + item.time;
			bsoncxx::stdx::optional<bsoncxx::document::value> found_item = data_coll.find_one(bsoncxx::builder::stream::document{} << "date_time" << date_time << finalize);
			// 같은 날짜가 없으면 새로 추가 한다.
			if (!found_item) {
				bsoncxx::document::value doc_chart_data_item = builder
					<< "date_time" << date_time
					<< "local_date" << item.date
					<< "local_time" << item.time
					<< "o" << item.o
					<< "h" << item.h
					<< "l" << item.l
					<< "c" << item.c
					<< "v" << item.v
					<< bsoncxx::builder::stream::finalize;
				auto res = db[data_key].insert_one(std::move(doc_chart_data_item));
			}
			else { // 같은 날짜가 있으면 업데이트 한다.
				data_coll.update_one(bsoncxx::builder::stream::document{} << "date_time" << date_time << finalize,
					bsoncxx::builder::stream::document{} << "$set"
					<< open_document
					<< "local_date" << item.date
					<< "local_time" << item.time
					<< "o" << item.o
					<< "h" << item.h
					<< "l" << item.l
					<< "c" << item.c
					<< "v" << item.v
					<< close_document << finalize);
			}
		}
	}
	catch (std::exception e) {
		std::string error;
		error = e.what();
	}
}

void SmMongoDBManager::SaveMarketsToDatabase()
{
	try
	{
		auto db = (*_Client)["andromeda"];
		using namespace bsoncxx;

		// 먼저 시장이 있는지 검색한다. 
		// 그리고 시장 속에 상품이 있는지 검색한다.
		mongocxx::collection coll = db["market_list"];
		
		builder::stream::document builder{};

		SmMarketManager* marketMgr = SmMarketManager::GetInstance();
		std::vector<SmMarket*>& marketList = marketMgr->GetMarketList();
		for (size_t i = 0; i < marketList.size(); ++i) {
			SmMarket* market = marketList[i];
			bsoncxx::stdx::optional<bsoncxx::document::value> found_market =
				coll.find_one(bsoncxx::builder::stream::document{} << "market_name" << SmUtfUtil::AnsiToUtf8((char*)market->Name().c_str()) << finalize);
			if (!found_market) {
				auto in_array = builder << "product_list" << builder::stream::open_array;
				std::vector<SmCategory*>& catVec = market->GetCategoryList();
				for (size_t j = 0; j < catVec.size(); ++j) {
					SmCategory* cat = catVec[j];
					bsoncxx::stdx::optional<bsoncxx::document::value> found_product =
						coll.find_one(bsoncxx::builder::stream::document{} << "prodcut_list.product_code" << cat->Code() << finalize);
					if (!found_product) {
						in_array = in_array << builder::stream::open_document
							<< "product_index" << (int)j
							<< "product_code" << cat->Code()
							<< "product_name_kr" << SmUtfUtil::AnsiToUtf8((char*)cat->NameKr().c_str())
							<< "product_name_en" << cat->Name()
							<< "exchange_name" << cat->Exchange()
							<< "exchange_code" << cat->ExchangeCode()
							<< "market_name" << SmUtfUtil::AnsiToUtf8((char*)cat->MarketName().c_str())
							<< builder::stream::close_document;
					}
				}
				auto after_array = in_array << builder::stream::close_array;
				after_array << "market_index" << (int)i
					<< "market_name" << SmUtfUtil::AnsiToUtf8((char*)market->Name().c_str());

			
				bsoncxx::document::value doc = after_array << builder::stream::finalize;

				bsoncxx::document::view view = doc.view();

				// Once we have the document view, we can use ["key"] or [index] notation to reach into nested
				// documents or arrays.
				auto awards = view["market_name"];

								
				std::string MyString = awards.get_utf8().value.to_string();

				std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8conv;
				std::wstring cv = utf8conv.from_bytes(MyString);

				std::string value;
				value = SmUtfUtil::unicode2ansi(cv);

				auto res = db["market_list"].insert_one(std::move(doc));

				
			}
		}

		
	}
	catch (std::exception e) {
		std::string error;
		error = e.what();
	}
}

void SmMongoDBManager::SaveSymbolsToDatabase()
{
	try
	{
		auto db = (*_Client)["andromeda"];
		using namespace bsoncxx;

		// 먼저 시장이 있는지 검색한다. 
		// 그리고 시장 속에 상품이 있는지 검색한다.
		mongocxx::collection coll = db["symbol_list"];

		builder::stream::document builder{};

		SmMarketManager* marketMgr = SmMarketManager::GetInstance();
		std::vector<SmMarket*>& marketList = marketMgr->GetMarketList();

		for (size_t i = 0; i < marketList.size(); ++i) {
			SmMarket* market = marketList[i];
			std::vector<SmCategory*>& cat_list = market->GetCategoryList();
			for (size_t j = 0; j < cat_list.size(); ++j) {
				SmCategory* cat = cat_list[j];
				std::vector<SmSymbol*>& sym_list = cat->GetSymbolList();
				for (size_t k = 0; k < sym_list.size(); ++k) {
					SmSymbol* sym = sym_list[k];
					bsoncxx::stdx::optional<bsoncxx::document::value> found_symbol =
						coll.find_one(bsoncxx::builder::stream::document{} << "symbol_code" << sym->SymbolCode() << finalize);
					if (!found_symbol) {
						bsoncxx::document::value doc_value = builder
							<< "symbol_code" << sym->SymbolCode()
							<< "symbol_index" << sym->Index()
							<< "symbol_name_kr" << SmUtfUtil::AnsiToUtf8((char*)sym->Name().c_str())
							<< "symbol_name_en" << sym->NameEn()
							<< "product_code" << sym->CategoryCode()
							<< "market_name" << SmUtfUtil::AnsiToUtf8((char*)sym->MarketName().c_str())
							<< "decimal" << sym->Decimal()
							<< "contract_unit" << sym->CtrUnit()
							<< "seungsu" << sym->Seungsu()
							<< "tick_size" << sym->TickSize()
							<< "tick_value" << sym->TickValue()
							<< bsoncxx::builder::stream::finalize;
						auto res = db["symbol_list"].insert_one(std::move(doc_value));
					}
				}
			}
		}
	}
	catch (std::exception e) {
		std::string error;
		error = e.what();
	}
}

void SmMongoDBManager::InitDatabase()
{
	_Instance = new mongocxx::instance();
	_Client = new mongocxx::client(mongocxx::uri{});
}
