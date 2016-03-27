#include "client_connection.h"
#include "client_exception.h"
#include "server.h"


ClientConnection::ClientConnection(boost::asio::io_service& service, Server::ptr server) 
	: _sock(service), _started(false), _stopped(true), _busy(false), _server(server) {
}

ClientConnection::ptr ClientConnection::new_(boost::asio::io_service& service, Server::ptr server) {
	ptr new_(new ClientConnection(service, server));
	return new_;
}

void ClientConnection::start() {
	_started = true;
	_stopped = false;
	do_read();
}

void ClientConnection::stop() {
	if (!_started) {
		return;
	}
	BOOST_LOG_TRIVIAL(info) << "Stopping client connection...";
	// TODO: send stop command to the client
	_started = false;
	if (!_busy) {
		_stopped = true;
		_sock.close();
	}
}

bool ClientConnection::is_stopped() {
	return _stopped;
}

ClientConnection::~ClientConnection() {
	BOOST_LOG_TRIVIAL(info) << "ClientConnection destruction...";
	stop();
}

void ClientConnection::do_read() {
	if (!_started) {
		_stopped = true;
		_sock.close();
		return;
	}
	async_read(_sock, buffer(_read_buffer),
		boost::bind(&ClientConnection::read_complete, shared_from_this(), _1, _2),
		boost::bind(&ClientConnection::on_read, shared_from_this(), _1, _2));
}

size_t ClientConnection::read_complete(const boost::system::error_code &err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "read_complete error: " << err;
		_busy = false;
		return 0;
	}
	_busy = bytes > 0;
	bool found = std::find(_read_buffer, _read_buffer + bytes, '\n') < _read_buffer + bytes;
	return found ? 0 : 1;
}


void ClientConnection::on_read(const boost::system::error_code &err, size_t bytes) {
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_read error: " << err;
		_busy = false;
		stop();
		return;
	}
	try {
		std::string msg(_read_buffer, bytes);
		BOOST_LOG_TRIVIAL(info) << "Received a msg: " << msg;
		double res = process_msg(msg);
		std::string resStr = std::to_string(res);
		do_write("Server response: " + resStr + "\n");
	}
	catch (...) {
		_busy = false;
		throw client_exception("Unexpected error in client connection", shared_from_this());
	}
}

double ClientConnection::process_msg(const std::string &msg) {
	int num = std::stoi(msg);
	_server.lock()->add_num(num);
	double res = _server.lock()->calc_res();
	return res;
}

void ClientConnection::do_write(const std::string & msg) {
	_busy = true;
	std::copy(msg.begin(), msg.end(), _write_buffer);
	_sock.async_write_some(buffer(_write_buffer, msg.size()),
		boost::bind(&ClientConnection::on_write, shared_from_this(), _1, _2));
}

void ClientConnection::on_write(const boost::system::error_code & err, size_t bytes) {
	_busy = false;
	if (err) {
		BOOST_LOG_TRIVIAL(error) << "on_write error: " << err;
		stop();
		return;
	}
	do_read();
}

ip::tcp::socket& ClientConnection::sock() {
	return _sock;
}

