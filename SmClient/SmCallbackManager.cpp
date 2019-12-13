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
	// �ʿ� �ִ� ��� �Լ��� ȣ���Ѵ�.
	for (auto it = _SymbolMap.begin(); it != _SymbolMap.end(); ++it) {
		it->second(symbol);
	}
}

