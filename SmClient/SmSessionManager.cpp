#include "SmSessionManager.h"
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include "SmSession.h"
#include "Json/json.hpp"
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
	loginInfo["reg_id"] = 1;
	loginInfo["user_info"]["id"] = _Session->Id();
	loginInfo["user_info"]["pwd"] = _Session->Pwd();
	Send(loginInfo.dump(4));
}
