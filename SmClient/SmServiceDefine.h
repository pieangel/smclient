#pragma once
enum class SmProtocol {
	req_none = 0,
	req_login,
	res_login,
	req_logout,
	res_logout,
	req_register_symbol,
	res_register_symbol,
	req_unregister_symbol,
	res_unregister_symbol,
	req_register_symbol_cycle,
	res_register_symbol_cycle,
	req_order_new,
	res_order_new,
	req_order_modify,
	res_order_modify,
	req_order_cancel,
	res_order_cancel,
	res_order_accepted,
	res_order_filled,
	req_chart_data,
	res_chart_data
};