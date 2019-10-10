#pragma once
#include <string>
#include <vector>
class SmSymbol;
class SmProductYearMonth
{
public:
	SmProductYearMonth();
	~SmProductYearMonth();
	std::string ProductCode;
	std::string YearMonthCode;
	std::vector<SmSymbol*> SymbolList;
};

