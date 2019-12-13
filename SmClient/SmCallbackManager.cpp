#include "pch.h"
#include "SmCallbackManager.h"


SmCallbackManager::SmCallbackManager()
{
}


SmCallbackManager::~SmCallbackManager()
{
}

void SmCallbackManager::OnSymbolEvent(const SmSymbol* symbol)
{
	if (!symbol)
		return;
	// 맵에 있는 모든 함수를 호출한다.
	for (auto it = _SymbolMap.begin(); it != _SymbolMap.end(); ++it) {
		it->second(symbol);
	}
}

