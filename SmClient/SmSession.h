#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

class SmSession : public std::enable_shared_from_this<SmSession>
{
private:
	tcp::resolver resolver_;
	websocket::stream<beast::tcp_stream> ws_;
	beast::flat_buffer buffer_;
	std::string host_;
	std::string id_;
	std::string pwd_;
	std::vector<boost::shared_ptr<std::string const>> queue_;
public:
	// Resolver and socket require an io_context
	explicit
		SmSession(net::io_context& ioc)
		: resolver_(net::make_strand(ioc))
		, ws_(net::make_strand(ioc))
	{
	}

	~SmSession();
	// Start the asynchronous operation
	void
		run(
			char const* host,
			char const* port,
			char const* id,
			char const* pwd
		);

	void
		on_resolve(
			beast::error_code ec,
			tcp::resolver::results_type results);

	void
		on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);

	void
		on_handshake(beast::error_code ec);

	void
		on_write(
			beast::error_code ec,
			std::size_t bytes_transferred);

	void do_read();

	void
		on_read(
			beast::error_code ec,
			std::size_t bytes_transferred);

	void
		on_close(beast::error_code ec);

	void send(boost::shared_ptr<std::string const> const& ss);
	void on_send(boost::shared_ptr<std::string const> const& ss);

	void close();
	std::string Id() const { return id_; }
	void Id(std::string val) { id_ = val; }
	std::string Pwd() const { return pwd_; }
	void Pwd(std::string val) { pwd_ = val; }
};

