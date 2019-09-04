#include "pch.h"
#include "SmSymbolManager.h"
#include "SmSymbol.h"

SmSymbolManager::SmSymbolManager()
{
}


SmSymbolManager::~SmSymbolManager()
{
	for (auto it = _SymbolMap.begin(); it != _SymbolMap.end(); ++it) {
		delete it->second;
	}
}

void SmSymbolManager::AddSymbol(SmSymbol* sym)
{
	if (!sym)
		return;

	_SymbolMap[sym->SymbolCode()] = sym;
}

SmSymbol* SmSymbolManager::FindSymbol(std::string symCode)
{
	auto it = _SymbolMap.find(symCode);
	if (it != _SymbolMap.end()) {
		return it->second;
	}

	return nullptr;
}
