#pragma once
#include <thread>
#include <vector>
//#include "SmSession.h"
class SmNetClient
{
	void Start();
public:
	SmNetClient() : _server_thread(&SmNetClient::ThreadMain, this) {}
	~SmNetClient() {
		_server_thread.join();
	}
private:
	std::thread _server_thread;
	void ThreadMain();
	// The io_context is required for all I/O
	//net::io_context ioc;
};

