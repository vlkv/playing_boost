#include "client.h"
#include "client_exception.h"
#include <string>
#include <ctime>
#include <boost/asio/ip/tcp.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <boost/thread.hpp>


Client::Client(std::string host, int port) : _host(host), _port(port), _sock(_service), _started(false), _gen(std::time(0)) {
}

Client::~Client() {
	BOOST_LOG_TRIVIAL(info) << "Client destruction...";
	stop();
}

int Client::gen_rand_num() {
	boost::random::uniform_int_distribution<> dist(0, 1023);
	return dist(_gen);
}

void Client::start() {
	BOOST_LOG_TRIVIAL(info) << "Client start";
	try {
		_started = true;
		connect();
		service_run_loop();
	}
	catch (const std::exception &e) {
		BOOST_LOG_TRIVIAL(error) << "Unexpected std::exception: " << e.what();
		stop();
	}
	catch (...) {
		BOOST_LOG_TRIVIAL(fatal) << "Unexpected unknown exception";
		stop();
	}
}

void Client::connect() {
	boost::system::error_code err;
	ip::address addr = ip::address::from_string(_host, err);
	if (err) {
		ostringstream oss;
		oss << "Bad host " << _host << ", reason:" << err;
		throw client_exception(oss.str());
	}
	ip::tcp::endpoint ep(addr, _port);
	_sock.async_connect(ep, boost::bind(&Client::on_connect, shared_from_this(), _1));
}

void Client::service_run_loop() {
	while (true) {
		try {
			_service.run();
		}
		// TODO: some exceptions could be not fatal... what are they?
		catch (...) {
			throw;
		}
	}
}

void Client::on_connect(const boost::system::error_code& err) {
	if (err) {
		ostringstream oss;
		oss << "on_connect error: " << err;
		throw client_exception(oss.str());
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
	BOOST_LOG_TRIVIAL(info) << "Stopped";
}

void Client::stop_async() {
	_service.dispatch(boost::bind(&Client::stop, shared_from_this()));
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
		ostringstream oss;
		oss << "on_write error: " << err;
		throw client_exception(oss.str());
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
		ostringstream oss;
		oss << "on_read error: " << err;
		throw client_exception(oss.str());
	}
	std::string msg(_read_buffer, bytes);
	BOOST_LOG_TRIVIAL(info) << "Received response: " << msg;

	send_rand_num();
}
