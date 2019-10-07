#pragma once
#include "Global/TemplateSingleton.h"

class SmLogManager : public TemplateSingleton<SmLogManager>
{
public:
	SmLogManager();
	~SmLogManager();
	void InitLog();
};

