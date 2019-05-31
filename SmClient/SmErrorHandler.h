#pragma once
#include "Global/TemplateSingleton.h"
class SmErrorHandler : public TemplateSingleton<SmErrorHandler>
{
public:
	SmErrorHandler();
	~SmErrorHandler();
	void ShowMessage(std::string msg);
};

