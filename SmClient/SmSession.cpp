#include "SmSession.h"
#include "SmSessionManager.h"
// Report a failure
void
fail(beast::error_code ec, char const* what)
{
	std::cerr << what << ": " << ec.message() << "\n";
}

SmSession::~SmSession()
{
	
}

void SmSession::run(char const* host, char const* port, char const* id, char const* pwd)
{
	SmSessionManager* sessMgr = SmSessionManager::GetInstance();
	sessMgr->Session(this);
	// Save these for later
	host_ = host;
	id_ = id;
	pwd_ = pwd;

	// Look up the domain name
	resolver_.async_resolve(
		host,
		port,
		beast::bind_front_handler(
			&SmSession::on_resolve,
			shared_from_this()));
}

void SmSession::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
	if (ec)
		return fail(ec, "resolve");

	// Set the timeout for the operation
	beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

	// Make the connection on the IP address we get from a lookup
	beast::get_lowest_layer(ws_).async_connect(
		results,
		beast::bind_front_handler(
			&SmSession::on_connect,
			shared_from_this()));
}

void SmSession::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
{
	if (ec)
		return fail(ec, "connect");

	// Turn off the timeout on the tcp_stream, because
	// the websocket stream has its own timeout system.
	beast::get_lowest_layer(ws_).expires_never();

	// Set suggested timeout settings for the websocket
	ws_.set_option(
		websocket::stream_base::timeout::suggested(
			beast::role_type::client));

	// Set a decorator to change the User-Agent of the handshake
	ws_.set_option(websocket::stream_base::decorator(
		[](websocket::request_type& req)
		{
			req.set(http::field::user_agent,
				std::string(BOOST_BEAST_VERSION_STRING) +
				" websocket-client-async");
		}));

	// Perform the websocket handshake
	ws_.async_handshake(host_, "/",
		beast::bind_front_handler(
			&SmSession::on_handshake,
			shared_from_this()));
}

void SmSession::on_handshake(beast::error_code ec)
{
	if (ec)
		return fail(ec, "handshake");

	do_read();
}

void SmSession::on_write(beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if (ec)
		return fail(ec, "write");

	// Remove the string from the queue
	queue_.erase(queue_.begin());

	// Send the next message if any
	if (!queue_.empty())
		ws_.async_write(
			net::buffer(*queue_.front()),
			beast::bind_front_handler(
				&SmSession::on_write,
				shared_from_this()));
}

void SmSession::do_read()
{
	// Read a message into our buffer
	ws_.async_read(
		buffer_,
		beast::bind_front_handler(
			&SmSession::on_read,
			shared_from_this()));
}

void SmSession::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if (ec)
		return fail(ec, "read");

	buffer_.consume(buffer_.size());

	do_read();
}

void SmSession::on_close(beast::error_code ec)
{
	if (ec)
		return fail(ec, "close");

	// If we get here then the connection is closed gracefully

	// The make_printable() function helps print a ConstBufferSequence
	std::cout << beast::make_printable(buffer_.data()) << std::endl;
}

void SmSession::send(boost::shared_ptr<std::string const> const& ss)
{
	// Post our work to the strand, this ensures
	// that the members of `this` will not be
	// accessed concurrently.

	net::post(
		ws_.get_executor(),
		beast::bind_front_handler(
			&SmSession::on_send,
			shared_from_this(),
			ss));
}

void SmSession::on_send(boost::shared_ptr<std::string const> const& ss)
{
	// Always add to queue
	queue_.push_back(ss);

	// Are we already writing?
	if (queue_.size() > 1)
		return;

	// We are not currently writing, so send this immediately
	ws_.async_write(
		net::buffer(*queue_.front()),
		beast::bind_front_handler(
			&SmSession::on_write,
			shared_from_this()));
}

void SmSession::close()
{
	// Close the WebSocket connection
	ws_.async_close(websocket::close_code::normal,
		beast::bind_front_handler(
			&SmSession::on_close,
			shared_from_this()));
}
