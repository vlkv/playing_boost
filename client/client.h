#pragma once
#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

using namespace boost::asio;
using namespace std;

class Client : public boost::enable_shared_from_this<Client> {
	boost::asio::io_service _service;
	std::string _host;
	int _port;
	ip::tcp::socket _sock;
	enum { max_msg = 1024 };
	char _read_buffer[max_msg];
	char _write_buffer[max_msg];
	bool _started;
	boost::random::mt19937 _gen;

public:
	Client(std::string host, int port);
	void start();
	
private:
	void on_connect(const boost::system::error_code& err);
	void stop();
	void send_rand_num();
	int gen_rand_num();
	void do_write(const std::string & msg);
	void on_write(const boost::system::error_code& err, size_t bytes);
	void do_read();
	size_t read_complete(const boost::system::error_code & err, size_t bytes);
	void on_read(const boost::system::error_code & err, size_t bytes);
};

