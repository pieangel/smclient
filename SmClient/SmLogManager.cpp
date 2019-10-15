#include "pch.h"
#include "SmLogManager.h"
#include "SmConfigManager.h"
#include "Util/VtStringUtil.h"
#include <filesystem>
#include <fstream>
#include "Log/loguru.hpp"
namespace fs = std::filesystem;

SmLogManager::SmLogManager()
{
}


SmLogManager::~SmLogManager()
{
}

void SmLogManager::InitLog()
{
	SmConfigManager* configMgr = SmConfigManager::GetInstance();
	std::string fileName = _T("SmClient.log");
	std::string appPath;
	appPath = configMgr->GetApplicationPath();
	appPath.append(_T("\\"));

	std::string dirName = appPath;

	// 오늘날짜 디렉토리 이름을 만든다.
	dirName.append(VtStringUtil::getTimeStr());
	if (!fs::exists(dirName)) { // 디렉토리가 존재하지 않을 경우
						   // 오늘 날짜로 디렉토리 생성
		fs::create_directory(dirName);
		// 그 디렉토리 밑에 파일을 만든다.
		dirName.append(_T("\\"));
		dirName.append(fileName);
		std::ofstream outfile(dirName);
		outfile.close();
	}
	appPath.append(VtStringUtil::getTimeStr());
	appPath.append(_T("\\"));
	appPath.append(fileName);

	// Put every log message in "everything.log":
	loguru::add_file(appPath.c_str(), loguru::Append, loguru::Verbosity_MAX);

	// Only log INFO, WARNING, ERROR and FATAL to "latest_readable.log":
	//loguru::add_file("latest_readable.log", loguru::Truncate, loguru::Verbosity_INFO);

	// Only show most relevant things on stderr:
	loguru::g_stderr_verbosity = 1;

	LOG_F(INFO, _T("Log manager was created!"));
}
