#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

using namespace boost::asio;
using namespace std;

class Server;

class ClientConnection : public boost::enable_shared_from_this<ClientConnection> {
	typedef ClientConnection self_type;
	
private:
	ip::tcp::socket _sock;
	enum { max_msg = 1024 };
	char _read_buffer[max_msg];
	char _write_buffer[max_msg];
	bool _started;
	boost::weak_ptr<Server> _server;

public:
	typedef boost::shared_ptr<ClientConnection> ptr;

	static ptr new_(boost::asio::io_service& service, boost::shared_ptr<Server> server);
	void start();
	ip::tcp::socket& sock();

private:
	ClientConnection(boost::asio::io_service& service, boost::shared_ptr<Server> server);

	void stop();

	void do_read();
	void on_read(const boost::system::error_code & err, size_t bytes);
	size_t read_complete(const boost::system::error_code & err, size_t bytes);
	
	void on_write(const boost::system::error_code & err, size_t bytes);
	void do_write(const std::string & msg);
	
	
};

