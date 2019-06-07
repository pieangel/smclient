#include "pch.h"
#include "SmErrorHandler.h"

SmErrorHandler::SmErrorHandler()
{

}

SmErrorHandler::~SmErrorHandler()
{

}

void SmErrorHandler::ShowMessage(std::string msg)
{
	CString strMsg;
	strMsg.Format("%s\n", msg.c_str());
	TRACE(strMsg.Left(120));
}
