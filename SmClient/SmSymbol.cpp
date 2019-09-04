#include "pch.h"
#include "SmSymbol.h"
#include "Json/json.hpp"
#include "SmServiceDefine.h"
#include "SmTimeSeriesDBManager.h"
using namespace nlohmann;
SmSymbol::SmSymbol()
{
}


SmSymbol::~SmSymbol()
{
}

std::string SmSymbol::GetQuoteByJson()
{
	json quote;
	quote["res_id"] = SmProtocol::res_realtime_sise;
	quote["symbol_code"] = _SymbolCode;
	quote["time"] = Quote.OriginTime;
	quote["high"] = Quote.High;
	quote["low"] = Quote.Low;
	quote["open"] = Quote.Open;
	quote["close"] = Quote.Close;
	quote["sign_to_preday"] = Quote.SignToPreDay;
	quote["gap_from_preday"] = Quote.GapFromPreDay;
	quote["ratio_to_preday"] = Quote.RatioToPreday;

	return quote.dump();
}

std::string SmSymbol::GetHogaByJson()
{
	json hoga;
	hoga["res_id"] = SmProtocol::res_realtime_hoga;
	hoga["symbol_code"] = _SymbolCode;
	hoga["time"] = Hoga.Time;
	hoga["domestic_date"] = Hoga.DomesticDate;
	hoga["domestic_time"] = Hoga.DomesticTime;
	hoga["tot_buy_qty"] = Hoga.TotBuyQty;
	hoga["tot_sell_qty"] = Hoga.TotSellQty;
	hoga["tot_buy_cnt"] = Hoga.TotBuyCnt;
	hoga["tot_sell_cnt"] = Hoga.TotSellCnt;

	for (int i = 0; i < 5; i++) {
		hoga["hoga_items"][i]["buy_price"] = Hoga.Ary[i].BuyPrice;
		hoga["hoga_items"][i]["buy_cnt"] = Hoga.Ary[i].BuyCnt;
		hoga["hoga_items"][i]["buy_qty"] = Hoga.Ary[i].BuyQty;
		hoga["hoga_items"][i]["sell_price"] = Hoga.Ary[i].SellPrice;
		hoga["hoga_items"][i]["sell_cnt"] = Hoga.Ary[i].SellCnt;
		hoga["hoga_items"][i]["sell_qty"] = Hoga.Ary[i].SellQty;
	}

	return hoga.dump();
}
