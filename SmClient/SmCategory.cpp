#include "pch.h"
#include "SmCategory.h"
#include "SmSymbol.h"
#include "SmSymbolManager.h"

SmCategory::SmCategory()
{
}


SmCategory::~SmCategory()
{
	
}

SmSymbol* SmCategory::AddSymbol(std::string symCode)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = new SmSymbol();
	sym->SymbolCode(symCode);
	_SymbolList.push_back(sym);
	symMgr->AddSymbol(sym);
	return sym;
}

SmSymbol* SmCategory::GetRecentMonthSymbol()
{
	if (_SymbolList.size() == 0)
		return nullptr;
	return _SymbolList[0];
}

