#pragma once
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <boost/log/trivial.hpp>

using namespace boost::asio;
using namespace std;

class Server;

class ClientConnection : public boost::enable_shared_from_this<ClientConnection> {
	typedef ClientConnection self_type;
	
private:
	int _id;
	static int _next_id;
	ip::tcp::socket _sock;
	enum { max_msg = 1024 };
	char _read_buffer[max_msg];
	char _write_buffer[max_msg];
	bool _started;
	bool _busy;
	bool _need_stop;
	boost::weak_ptr<Server> _server;

public:
	typedef boost::shared_ptr<ClientConnection> ptr;

	static ptr new_(boost::asio::io_service& service, boost::shared_ptr<Server> server);
	void start();
	void stop();
	void disconnect();
	bool is_stopped();
	virtual ~ClientConnection();
	ip::tcp::socket& sock();
	const int id() const;

private:
	ClientConnection(boost::asio::io_service& service, boost::shared_ptr<Server> server);

	void do_read_write();
	void do_read();
	void on_read(const boost::system::error_code & err, size_t bytes);
	size_t read_complete(const boost::system::error_code & err, size_t bytes);
	
	void do_write(const std::string &msg);
	void on_write(const boost::system::error_code & err, size_t bytes);

	void do_write_disconnected();
	void on_write_disconnected(const boost::system::error_code & err, size_t bytes);

	void do_write_stop();
	
	void handle_msg(const std::string &msg);
	double process_num(int num);
	
	

	void stop_sock_close();
	
};