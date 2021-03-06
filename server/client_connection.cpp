#include "client_connection.h"
#include "server.h"
#include "server_exceptions.h"
#include <boost/algorithm/string.hpp>

int ClientConnection::_next_id = 1;


ClientConnection::ClientConnection(boost::asio::io_service& service, Server::ptr server) 
	: _id(ClientConnection::_next_id++), _sock(service), _server(server), _started(false), _need_stop(false) {
}

ClientConnection::ptr ClientConnection::new_(boost::asio::io_service& service, boost::shared_ptr<Server> server) {
	ptr c(new ClientConnection(service, server));
	return c;
}

void ClientConnection::start() {
	BOOST_LOG_TRIVIAL(info) << "Started ClientConnection id=" << _id;
	_started = true;
	do_read();
}

void ClientConnection::stop() {
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping ClientConnection id=" << _id << "...";
	_started = false;
	_need_stop = true;
}

void ClientConnection::do_read() {
	async_read(_sock, buffer(_read_buffer),
		boost::bind(&ClientConnection::read_complete, shared_from_this(), _1, _2),
		boost::bind(&ClientConnection::on_read, shared_from_this(), _1, _2));
}

size_t ClientConnection::read_complete(const boost::system::error_code &err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "read_complete error: " << err << " client id=" << _id;
		return 0;
	}
	bool found = std::find(_read_buffer, _read_buffer + bytes, '\n') < _read_buffer + bytes;
	return found ? 0 : 1;
}

void ClientConnection::on_read(const boost::system::error_code &err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_read error: " << err << " client id=" << _id;
		throw server_exception("on_read error", shared_from_this());
	}
	std::string msg(_read_buffer, bytes);
	BOOST_LOG_TRIVIAL(info) << "Received from client id=" << _id << " msg: " << msg;
	if (_need_stop) {
		_need_stop = false;
		do_write("stop\n", boost::bind(&ClientConnection::on_write_disconnected, shared_from_this(), _1, _2));
	}
	else {
		handle_msg(msg);
	}
}


void ClientConnection::handle_msg(const std::string &msg) {
	std::vector<std::string> strs;
	boost::split(strs, msg, boost::is_any_of(":\n"));
	std::string cmd = strs[0];
	if (cmd == "num") {
		std::string numStr = strs[1];
		int num = std::stoi(numStr);
		double res = process_num(num);
		std::string resStr = std::to_string(res);
		do_write("ok:" + resStr + "\n", boost::bind(&ClientConnection::on_write_ok, shared_from_this(), _1, _2));
	}
	else if (cmd == "disconnect") {
		do_write("disconnected\n", boost::bind(&ClientConnection::on_write_disconnected, shared_from_this(), _1, _2));
	}
	else {
		std::ostringstream oss;
		oss << "Unexpected msg from client: " << msg;
		throw server_exception(oss.str(), shared_from_this());
	}
}

double ClientConnection::process_num(int num) {
	_server.lock()->add_num(num);
	double res = _server.lock()->calc_res();
	return res;
}


void ClientConnection::do_write(const std::string &msg, OnWriteHandler on_write_handler) {
	BOOST_LOG_TRIVIAL(info) << "Sending to client id=" << _id << " msg: " << msg;
	std::copy(msg.begin(), msg.end(), _write_buffer);
	boost::asio::async_write(_sock, buffer(_write_buffer, msg.size()), on_write_handler);
}

void ClientConnection::on_write_ok(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		std::ostringstream oss;
		oss << "on_write error: " << err << " client id=" << _id;
		throw server_exception(oss.str(), shared_from_this());
	}
	do_read();
}

void ClientConnection::on_write_disconnected(const boost::system::error_code & err, size_t bytes) {
	if (err) {
		std::ostringstream oss;
		oss << "on_write_disconnected error: " << err << " client id=" << _id;
		throw server_exception(oss.str(), shared_from_this());
	}
	_sock.close();
	throw disconnected_exception(shared_from_this());
}

ip::tcp::socket& ClientConnection::sock() {
	return _sock;
}

const int ClientConnection::id() const {
	return _id;
}

ClientConnection::~ClientConnection() {
	try {
		BOOST_LOG_TRIVIAL(info) << "ClientConnection id=" << _id << " destruction...";
		if (_sock.is_open()) {
			_sock.close();
		}
	}
	catch (...) {}
}
