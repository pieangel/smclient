#pragma once
#include <map>
#include <functional>
#include "Global/TemplateSingleton.h"

class SmSymbol;
class SmCallbackManager : public TemplateSingleton<SmCallbackManager>
{
public:
	SmCallbackManager();
	~SmCallbackManager();
	void SubscribeSymbolCallback(long id, std::function <void(const SmSymbol * symbol)> callback) {
		_SymbolMap[id] = callback;
	}
	void OnSymbolEvent(const SmSymbol* symbol);
private:
	// 심볼과 관련된 콜백을 저장하는 맵
	std::map<long, std::function<void(const SmSymbol * symbol)>> _SymbolMap;
};

