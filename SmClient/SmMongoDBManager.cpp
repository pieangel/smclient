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

void SmMongoDBManager::InitDatabase()
{
	_Instance = new mongocxx::instance();
	_Client = new mongocxx::client(mongocxx::uri{});
}
