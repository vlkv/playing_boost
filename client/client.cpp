#include "client.h"
#include <string>
#include <ctime>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>
#include <iostream>


Client::Client(std::string host, int port) : _host(host), _port(port), _sock(_service), _started(false), _gen(std::time(0)) {
}

int Client::gen_rand_num() {
	boost::random::uniform_int_distribution<> dist(0, 1023);
	return dist(_gen);
}

void Client::start() {
	ip::tcp::endpoint ep(ip::address::from_string(_host), _port);
	_sock.async_connect(ep, boost::bind(&Client::on_connect, shared_from_this(), _1));
	_started = true;
	_service.run();
}

void Client::on_connect(const boost::system::error_code& err) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_connect error: " << err;
		stop();
	}
	BOOST_LOG_TRIVIAL(info) << "Connected!";
	send_rand_num();
}

void Client::send_rand_num() {
	int rand_num = gen_rand_num();
	string str = std::to_string(rand_num);
	BOOST_LOG_TRIVIAL(info) << "Sending request: " << str;
	do_write(str + "\n");
}

void Client::stop() {
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping...";
	_started = false;
	_sock.close();
}

void Client::do_write(const std::string& msg) {
	if (!_started) {
		return;
	}
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&Client::on_write, shared_from_this(), _1, _2));
}

void Client::on_write(const boost::system::error_code& err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_write error: " << err;
		stop();
	}
	do_read();
}

void Client::do_read() {
	async_read(_sock, buffer(_read_buffer),
		boost::bind(&Client::read_complete, shared_from_this(), _1, _2), 
		boost::bind(&Client::on_read, shared_from_this(),  _1, _2));
}

size_t Client::read_complete(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "read_complete error: " << err;
		return 0;
	}
	bool found = std::find(_read_buffer, _read_buffer + bytes, '\n') < _read_buffer + bytes;
	return found ? 0 : 1;
}

void Client::on_read(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_read error: " << err;
		stop();
	}
	if (!_started) {
		return;
	}
	std::string msg(_read_buffer, bytes);
	BOOST_LOG_TRIVIAL(info) << "Received response: " << msg;

	send_rand_num();
}
