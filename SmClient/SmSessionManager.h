#pragma once
#include "Global/TemplateSingleton.h"
#include <string>
class SmSession;
class SmSessionManager : public TemplateSingleton<SmSessionManager>
{
public:
	SmSessionManager();
	~SmSessionManager();
	void Send(std::string msg);
	SmSession* Session() const { return _Session; }
	void Session(SmSession* val) { _Session = val; }
	void Login();
	void RegisterProduct(std::string symCode);
	void OnMessage(std::string message);
private:
	SmSession* _Session = nullptr;
};

