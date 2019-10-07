#include "pch.h"
#include "SmProduct.h"
#include "SmSymbol.h"
#include "SmSymbolManager.h"

SmProduct::SmProduct()
{
}


SmProduct::~SmProduct()
{
	
}

SmSymbol* SmProduct::AddSymbol(std::string symCode)
{
	SmSymbolManager* symMgr = SmSymbolManager::GetInstance();
	SmSymbol* sym = new SmSymbol();
	sym->SymbolCode(symCode);
	_SymbolList.push_back(sym);
	symMgr->AddSymbol(sym);
	return sym;
}

SmSymbol* SmProduct::GetRecentMonthSymbol()
{
	if (_SymbolList.size() == 0)
		return nullptr;
	return _SymbolList[0];
}

