#pragma once
#include <thread>
#include <vector>
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
};

