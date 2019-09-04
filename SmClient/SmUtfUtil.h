#pragma once
#include <string>
class SmUtfUtil
{
public:
	static std::string AnsiToUtf8(char* ansi);
	static std::string Utf8ToAnsi(char* utf8);
};

