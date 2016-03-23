#pragma once
#include "client_connection.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <list>

using namespace boost::asio;

class Server : public boost::enable_shared_from_this<Server> {
	io_service _service;
	ip::tcp::acceptor _acceptor;
	std::list<ClientConnection::ptr> _clients;

public:
	typedef boost::shared_ptr<Server> ptr;

	Server(int port);
	void start();
	void Server::accept_client();
	void on_accept(ClientConnection::ptr client, const boost::system::error_code& err);
};

