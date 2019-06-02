#include "SmSessionManager.h"
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include "SmSession.h"
#include "Json/json.hpp"
#include "SmErrorHandler.h"
using namespace nlohmann;
SmSessionManager::SmSessionManager()
{

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

	json loginInfo;
	loginInfo["req_id"] = 1;
	loginInfo["user_info"]["id"] = _Session->Id();
	loginInfo["user_info"]["pwd"] = _Session->Pwd();
	Send(loginInfo.dump(4));
}



void SmSessionManager::RegisterProduct(std::string symCode)
{
	if (!_Session)
		return;
	json reg_symbol;
	reg_symbol["req_id"] = 2;
	reg_symbol["user_id"] = _Session->Id();
	reg_symbol["symbol_code"] = symCode;
	_RegSymbolSet.insert(symCode);
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
