#include "pch.h"
#include "SmUtfUtil.h"

std::string SmUtfUtil::AnsiToUtf8(char* ansi)
{
	WCHAR unicode[1500];
	char utf8[1500];

	memset(unicode, 0, sizeof(unicode));
	memset(utf8, 0, sizeof(utf8));

	::MultiByteToWideChar(CP_ACP, 0, ansi, -1, unicode, sizeof(unicode));
	::WideCharToMultiByte(CP_UTF8, 0, unicode, -1, utf8, sizeof(utf8), NULL, NULL);

	return std::string(utf8);
}

std::string SmUtfUtil::Utf8ToAnsi(char* utf8)
{
	WCHAR* unicode = NULL;
	char* ansi = NULL;

	unicode = (WCHAR*)calloc(1, strlen(utf8) * 2);
	::MultiByteToWideChar(CP_UTF8, 0, utf8, -1, unicode, strlen(utf8) * 2);

	//ansi = (char*)calloc(1, _tcslen(unicode)*2);
	ansi = (char*)calloc(1, wcslen(unicode) * 2);  /* 테스트한 프로젝트의 문자 집합 속성이 MultiByte 이므로 _tcslen()은 strlen()으로 정의된다. strlen()은 char*을 인자로 받으므로 컴파일 오류가 생긴다. */
	//::WideCharToMultiByte(CP_ACP, 0, unicode, -1, ansi, _tcslen(unicode)*2, NULL, NULL); 
	::WideCharToMultiByte(CP_ACP, 0, unicode, -1, ansi, wcslen(unicode) * 2, NULL, NULL);

	return std::string(ansi);
}
