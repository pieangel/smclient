#pragma once
#include "Global/TemplateSingleton.h"
#include <string>
#include <map>
class SmSymbol;
class SmSymbolManager : public TemplateSingleton<SmSymbolManager>
{
public:
	SmSymbolManager();
	~SmSymbolManager();
	void AddSymbol(SmSymbol* sym);
	SmSymbol* FindSymbol(std::string symCode);
	std::map<std::string, SmSymbol*>& GetSymbolMap() {
		return _SymbolMap;
	}
private:
	std::map<std::string, SmSymbol*> _SymbolMap;
};

