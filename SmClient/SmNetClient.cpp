#include "SmNetClient.h"
#include "SmSession.h"
void SmNetClient::ThreadMain()
{
	Start();
}

void SmNetClient::Start()
{
	auto const host = "192.168.0.38";
	//auto const host = "angelpie.ddns.net";
	auto const port = "9991";
	auto const id = "angelpie";
	auto const pwd = "orion1";

	// The io_context is required for all I/O
	net::io_context ioc;

	// Launch the asynchronous operation
	std::make_shared<SmSession>(ioc)->run(host, port, id, pwd);

	// Run the I/O service. The call will return when
	// the socket is closed.
	ioc.run();
}

