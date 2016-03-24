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
		cerr << "error! " << err << endl;
		stop();
	}
	cout << "Connected!" << endl;
	send_rand_num();
}

void Client::send_rand_num() {
	int rand_num = gen_rand_num();
	string str = std::to_string(rand_num);
	do_write(str + "\n");
}

void Client::stop() {
	if (!_started) return;
	std::cout << "stopping client..." << std::endl;
	_started = false;
	_sock.close();
}

void Client::do_write(const std::string& msg) {
	if (!_started) return;
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&Client::on_write, shared_from_this(), _1, _2));
}

void Client::on_write(const boost::system::error_code& err, size_t bytes) {
	cout << "Written " << bytes << " bytes" << endl;
	do_read();
}

void Client::do_read() {
	async_read(_sock, buffer(_read_buffer),
		boost::bind(&Client::read_complete, shared_from_this(), _1, _2), 
		boost::bind(&Client::on_read, shared_from_this(),  _1, _2));
}

size_t Client::read_complete(const boost::system::error_code & err, size_t bytes) {
	if (err) return 0;
	bool found = std::find(_read_buffer, _read_buffer + bytes, '\n') < _read_buffer + bytes;
	return found ? 0 : 1;
}

void Client::on_read(const boost::system::error_code & err, size_t bytes) {
	cout << "on_read" << endl;
	if (err) stop();
	if (!_started) return;
	std::string msg(_read_buffer, bytes);
	std::cout << "Received a msg: " << msg << std::endl;
	send_rand_num();
}