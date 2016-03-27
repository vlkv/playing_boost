#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <boost/log/trivial.hpp>
#include <boost/function.hpp>


using namespace boost::asio;
using namespace std;

class Server;

class ClientConnection : public boost::enable_shared_from_this<ClientConnection> {
	int _id;
	static int _next_id;
	ip::tcp::socket _sock;
	enum { max_msg = 1024 };
	char _read_buffer[max_msg];
	char _write_buffer[max_msg];
	bool _started;
	bool _need_stop;
	boost::weak_ptr<Server> _server;

public:
	typedef boost::shared_ptr<ClientConnection> ptr;

	static ptr new_(boost::asio::io_service& service, boost::shared_ptr<Server> server);
	void start();
	void stop();
	
	ip::tcp::socket& sock();
	const int id() const;
	virtual ~ClientConnection();

private:
	ClientConnection(boost::asio::io_service& service, boost::shared_ptr<Server> server);

	void do_read();
	size_t read_complete(const boost::system::error_code & err, size_t bytes);
	void on_read(const boost::system::error_code & err, size_t bytes);
	
	typedef boost::function<void(const boost::system::error_code &, size_t)> OnWriteHandler;
	void do_write(const std::string &msg, OnWriteHandler on_write_handler);
	void on_write_ok(const boost::system::error_code & err, size_t bytes);
	void on_write_disconnected(const boost::system::error_code & err, size_t bytes);

	void handle_msg(const std::string &msg);
	double process_num(int num);
};