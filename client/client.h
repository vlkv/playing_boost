#pragma once
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/log/trivial.hpp>
#include <boost/function.hpp>

using namespace boost::asio;

class Client : public boost::enable_shared_from_this<Client> {
	std::string _host;
	int _port;
	boost::asio::io_service _service;
	boost::asio::ip::tcp::socket _sock;
	
	enum { max_msg = 1024 };
	char _read_buffer[max_msg];
	char _write_buffer[max_msg];
	bool _started;
	bool _need_disconnect;
	boost::random::mt19937 _gen;

public:
	Client(std::string host, int port);
	void start();
	void stop_async();
	virtual ~Client();
	
private:
	void connect();
	void on_connect(const boost::system::error_code& err);
	void stop();

	void send_rand_num();
	void send_disconnect();

	typedef boost::function<void(const boost::system::error_code &, size_t)> OnWriteHandler;
	void do_write(const std::string &msg, OnWriteHandler on_write_handler);
	void on_write_num(const boost::system::error_code& err, size_t bytes);
	void on_write_disconnect(const boost::system::error_code& err, size_t bytes);
	
	void do_read();
	size_t read_complete(const boost::system::error_code & err, size_t bytes);
	void on_read(const boost::system::error_code & err, size_t bytes);
	
	void handle_msg(const std::string &msg);

	int gen_rand_num();
};

