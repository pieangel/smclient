#include "SmNetClient.h"
#include "SmWebsocketSession.h"
#include "SmSessionManager.h"
#include <string>
#include "SmConfigManager.h"
#include "Xml/pugixml.hpp"
void SmNetClient::ThreadMain()
{
	Start();
}

void SmNetClient::Start()
{
	SmConfigManager* configMgr = SmConfigManager::GetInstance();
	std::string appPath = configMgr->GetApplicationPath();
	std::string configPath = appPath;
	configPath.append("\\Config\\Config.xml");
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(configPath.c_str());
	pugi::xml_node app = doc.first_child();
	pugi::xml_node server_info = doc.child("application").child("server_info");

	std::string addr = server_info.child("ip").text().as_string();
	std::string port = server_info.child("port").text().as_string();
	std::string root_path = appPath; // server_info.child("root_path").text().as_string();
	std::string thread_count = server_info.child("thread_count").text().as_string();


	auto const id = "sise_server";
	auto const pwd = "orion1";

	// The io_context is required for all I/O
	net::io_context ioc;

	// Launch the asynchronous operation
	std::make_shared<SmWebsocketSession>(ioc)->run(addr.c_str(), port.c_str(), id, pwd);

	// Run the I/O service. The call will return when
	// the socket is closed.
	ioc.run();
}

